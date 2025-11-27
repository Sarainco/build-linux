#### VS800 System Build

##### 环境

- ubuntu 22.04
- 镜像源： http://mirrors.aliyun.com/ubuntu

##### 安装依赖软件包

```bash
sudo apt-get update
sudo apt-get upgrade
sudo apt-get install curl python2.7 git ssh make gcc libssl-dev liblz4-tool expect \
g++ patchelf chrpath gawk texinfo chrpath diffstat binfmt-support qemu-user-static live-build bison \
flex fakeroot cmake gcc-multilib g++-multilib unzip device-tree-compiler python3-pip \
libncurses-dev python3-pyelftools vim mtd-utils python2

#将python2设置为系统默认pathon版本
sudo rm -rf /usr/bin/python
sudo ln -s /usr/bin/python2.7 /usr/bin/python
```

##### git配置

```bash
git config --global user.name "Sarainco"
git config --global user.email "sarainco.sun@blztech.com"
```

##### sdk源码还原

```bash
#请将rk3568_dl源码包和该项目包放置于同一目录下；rk3568_dl中包含sdk源码包和第三方工具包
#示例：
yuji@linux:~/workspace$ ll
总计 16
drwxrwxrwx  4 yuji yuji 4096  8月 16 15:59 ./
drwxr-x--- 16 yuji yuji 4096  8月 16 16:18 ../
drwxrwxr-x  2 yuji yuji 4096  8月 16 16:04 rk3568_dl/
drwxr-xr-x  5 yuji yuji 4096  8月 16 15:46 vs800-linux-build/
#执行
cd vs800-linux-build
make sdk-prepare
```

##### 配置文件

```bash
cd vs800-linux-build
make envset
#第一次会出现交互信息，选择BoardConfig-rk3568-atk-atompi-ca1.mk对应的数字（3）；后续直接执行该命令就可以
```

##### uboot开发

```bash
cd vs800-linux-build
make uboot/menuconfig #通过 menuconfig 图形化界面、可自行对 U-Boot 进行配置；需要修改执行，其它情况直接忽略
make uboot #uboot编译

#示例：
# 启用U-Boot HW-ID DTB功能
# make uboot/menuconfig (ARM architecture ---> Enable support for selecting DTB by hardware id)点击y选中后save保存
# make uboot 执行该命令编译

#注：如果只涉及uboot设备树源码的修改，只需要执行make uboot即可
```

##### kernel开发

```bash
cd vs800-linux-build
make kernel/menuconfig #通过 menuconfig 图形化界面、可自行对 kernel 进行配置；需要修改执行，其它情况直接忽略
make kernel #kernel编译

#内核设备树文件路径为：
vs800-linux-build/stating/source/rk3568_linux_sdk/kernel/arch/arm64/boot/dts/rockchip/rk3568-atk-atompi-ca1.dts #原始屏
vs800-linux-build/stating/source/rk3568_linux_sdk/kernel/arch/arm64/boot/dts/rockchip/rk3568-atk-atompi-ca1-1024p.dts #中性屏

#设备树文件包含了产品使用的所有驱动对应的硬件信息，如果不确定使用的那个驱动，可以根据设备树节点找到使用的驱动源码
#示例：
# 屏幕触摸芯片的结点为：
#设备树结点参考绑定文档，路径Documentation/devicetree/bindings/input/touchscreen/edt-ft5x06.txt
&i2c1 {
	status = "okay";

	ft5436: edt-ft5x06@38 {
        compatible = "edt,edt-ft5x06";
        reg = <0x38>;
        pinctrl-names = "default";
        pinctrl-0 = <&touch_gpio>;
        interrupt-parent = <&gpio0>;
        interrupts = <RK_PC7 IRQ_TYPE_EDGE_FALLING>;
        reset-gpio = <&gpio0 RK_PC1 GPIO_ACTIVE_LOW>;
        touchscreen-size-x = <240>;
        touchscreen-size-y = <320>;
        status = "okay";
    };

   

};
#设备树和驱动是通过compatible属性进行匹配的，上述示例中compatible = "edt,edt-ft5x06"，可以在内核源码中通过该属性确定使用那个驱动
yuji@linux:~/workspace/vs800-linux-build/stating/source/rk3568_linux_sdk/kernel/drivers$ grep -nwr "edt,edt-ft5x06"
input/touchscreen/edt-ft5x06.c:1173:    { .compatible = "edt,edt-ft5x06", .data = &edt_ft5x06_data },
#上述c文件edt-ft5x06.c就是该芯片使用的驱动
```

##### rootfs开发

```bash
cd vs800-linux-build
make buildroot/menuconfig #通过 menuconfig 图形化界面、可自行对 buildroot 进行配置；需要修改执行，其它情况直接忽略
make buildroot #buildroot编译
```

##### 打包镜像

```bash
cd vs800-linux-build
make image #生成的镜像文件在stating/build/image/
```

##### 全量编译

```bash
cd vs800-linux-build
make all
```

##### 升级

```bash
#修改版本号：修改vs800-linux-build下gen_version.sh脚本中的UBOOT_VER、KERNEL_VER以及FS_VER变量

# 升级主要是构建升级包,系统脚本和业务程序路径在vs800-linux-build/buildroot/fs-overlay、内核镜像在vs800-linux-build/stating/image目录
# 文件系统脚本直接放在对应位置即可（eg:S22userapp脚本在文件系统/etc/init.d目录下，需要更新，直接将新文件放在项目包vs800-linux-build/buildroot/fs-overlay/etc/init.d）
# 业务程序的升级直接把新的可执行程序vs800_app放在vs800-linux-build/buildroot/fs-overlay/opt/upgrade目录，新的动态库libvs800_algorithm.so放在vs800-linux-build/buildroot/fs-overlay/opt/upgrade/lib目录
# 内核镜像由编译规则自动拷贝，不用手动管理
```



#### 附录

```bash
vs800-linux-build/buildroot/fs-overlay/usr/bin/upgrade.sh # 升级脚本
vs800-linux-build/buildroot/fs-overlay/etc/init.d/S22userapp #用户程序开机启动脚本
#!/bin/sh

APP=vs800_ctl
GPIO=128

case "$1" in
  start)
    echo "Configuring userapp..."

    # rtc link
    rm -f /dev/rtc
    ln -s /dev/rtc1 /dev/rtc
    hwclock -s -l

    # gpio
    [ ! -d /sys/class/gpio/gpio$GPIO ] && echo $GPIO > /sys/class/gpio/export
    echo out > /sys/class/gpio/gpio$GPIO/direction

    # npu freq
    # echo 900000000 > /sys/class/devfreq/fde40000.npu/min_freq

    # start app
    $APP start

    echo "userapp start done"
    ;;
  stop)
    echo "Stopping userapp..."
    $APP stop
    echo 0 > /sys/bus/i2c/devices/2-001b/proj_power
    /opt/apps/drm_png /opt/apps/vs-off.png
    ;;
  restart)
    $0 stop
    $0 start
    ;;
  *)
    echo "Usage: $0 {start|stop|restart}"
    exit 1
    ;;
esac
exit 0
#其中start开机执行，stop关机执行

vs800-linux-build/buildroot/fs-overlay/usr/lib/udev/ums_hotplug.sh #usb热插拔挂载脚本

vs800-linux-build/tools/ft5x06-tool # 触摸芯片FT5446升级工具
LDFLAGS=-static CC=/opt/atk-dlrk356x-toolchain/bin/aarch64-buildroot-linux-gnu-gcc make # 触摸芯片FT5446升级工具编译规则
```

