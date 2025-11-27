### PMOC100 System Build

PMOC100 RK3562 Linux 系统构建

#### 开发环境

unubtu22.04

#### 安装依赖

```bash
sudo apt update
sudo apt install git make gcc build-essential libncurses-dev bison flex libssl-dev lz4 u-boot-tools libtool libsysfs-dev texinfo meson cmake pkg-config gperf ninja-build mesa-common-dev libgl1-mesa-dev libglu1-mesa-dev

whereis python3
#python3: /usr/bin/python3 /usr/lib/python3 /etc/python3 /usr/share/python3 /usr/share/man/man1/python3.1.gz
ln -s /usr/bin/python3 /usr/bin/python
```

####  获取第三方工具包源码

```bash
git clone git@gitee.com:yuanjianpeng/tarballs.git
```

#### 获取rk3562sdk源码

```bash
git clone git@gitee.com:yuanjianpeng/zynqmp.git
```

#### 配置第三方工具包路径

需要保证用tarballs、toolchain、zynqmp三个工程目录；除toolchain外，其他可修改，但需要保证tarballs和(~/tarballs) tarball directory一致

```bash
make M=rk3562 menuconfig
# Generic options --->
 (~/tarballs) tarball directory
```

#### 工程配置

```bash
make M=rk3562 menuconfig #选择需要参与编译的工程组件
```

#### 全量编译编译

```bash
make M=rk3562 -j8 #目前yuan提供的包全量编译在kernel/install阶段编译不通过，没有给出解决方案
```

#### 单独编译

单独编译组件内容之前，需要先编译host和toolchain

```bash
make M=rk3562 host -j8; make M=rk3562 toolchain -j8
```

##### 单独编译kernel

```bash
make M=rk3562 kernel/menuconfig
make M=rk3562 kernel -j8
```

##### 单独编译app

```bash
make M=rk3562 app -j8;#编译app中的所有组件
make M=rk3562 app/qt6 -j8; #编译app中的qt6组件
```

##### 单独编译target

```bash
make M=rk3562 target -j8；#打包整个镜像
make M=rk3562 target/ext4fs -j8;#打包文件系统
```

#### 目标文件路径

```bash
~/zynqmp/staging/rk3562/image/#内核镜像和文件系统都在这个目录
```

#### 编译MOC200业务源码

获取MOC200源码并且进入工程目录

```bash
#指定Qt6_DIR路径
set(Qt6_DIR "~/zynqmp/staging/rk3562/app/qt-everywhere-src-6.4.3/staging/lib/cmake/Qt6")
```

新建build目录并进入该目录进行编译

```bash
~/zynqmp/staging/rk3562/app/qt6/bin/qt-cmake ..
make -j8

#若是出现aarch64-none-linux-gnu-g++: fatal error: Killed signal terminated program cc1plus错误，可通过修改swap分区解决
sudo swapoff /swapfile
sudo fallocate -l 4G /swapfile
sudo chmod 600 /swapfile
sudo mkswap /swapfile
sudo swapon /swapfile
#可以通过free命令查看是否修改成功
free -h
#修改完成后，删除build目录，重新编译
```

运行

```bash
export LD_LIBRARY_PATH=/usr/lib:/opt/qt6/lib
./MOSeriesClients -platform eglfs
```

#### uboot开发

```bash
tar -zxf u-boot-*.tar.gz #解压

# 编译
export PATH=/home/rain/toolchain/toolchain-rk3562/bin:$PATH
make rk3562_defconfig
./make.sh CROSS_COMPILE=aarch64-none-linux-gnu- rk3562 --spl-new
```

#### 升级

```bash
# 因为业务程序有专门的升级处理动作。该升级只涉及到系统脚本和内核镜像
# 内核镜像在zynqmp/staging/rk3562/image目录，由编译规则自动拷贝，不用手动管理
# 将需要更新的脚本放在zynqmp/targets/rk3562/fs-overlay相应目录，打包即可
```

#### PMOC100外设调试

##### 时钟芯片RTC

```bash
# 查看当前硬件时钟时间
hwclock -r

#设置系统时间
date -s "2025-06-30 10:31:56"
#将系统时间写入RTC
hwclock -w

###############
#往时钟芯片设置的时间为UTC时间，系统时间（CST）为UTC + 8;
##############
#如果往RTC设置的时间为本地系统时间，则需要强制告诉 hwclock RTC 是 localtime
hwclock -s -l

#默认使用12小时制，如需24小时制
export LC_TIME=C
```

##### 光报警TLC591x

```bash
#节点sys/class/leds
/ # cat /sys/class/leds/
disk/       ind0/       ind2/       ind4/       mmc0::/     user-led1/
heartbeat/  ind1/       ind3/       ind5/       user-led0/
#两个ind节点控制一路led灯，两路背光值最大，亮度最高
#ind0 && ind1 ----->red
#ind2 && ind3 ----->yellow
#ind4 && ind5 ----->blue

#关闭led灯
echo 0 > /sys/class/leds/ind0/brightness
echo 0 > /sys/class/leds/ind1/brightness
#亮度最大
echo 255 > /sys/class/leds/ind0/brightness
echo 255 > /sys/class/leds/ind1/brightness
#其它节点操作方式一样
```

##### 声音报警DIP2X5

```bash
echo 17 > /sys/class/gpio/export#导出该IO
echo out > /sys/class/gpio/gpio17/direction#设置为输出
echo 1 > /sys/class/gpio/gpio17/value#拉高
echo 0 > /sys/class/gpio/gpio17/value#拉低
#可以不断拉高拉低来模拟所需要的时序
echo 17 > /sys/class/gpio/unexport#取消导出，可选
```

```c
/*******参考代码********/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>

#define GPIO_NUM "17"   // gpio0_c1 编号17

void delay_us(int us) {
    struct timespec ts;
    ts.tv_sec = us / 1000000;
    ts.tv_nsec = (us % 1000000) * 1000;
    nanosleep(&ts, NULL);
}

int write_str_to_file(const char *path, const char *str) {
    int fd = open(path, O_WRONLY);
    if (fd < 0) {
        perror(path);
        return -1;
    }
    if (write(fd, str, strlen(str)) < 0) {
        perror("write");
        close(fd);
        return -1;
    }
    close(fd);
    return 0;
}

int main() {
    // 导出 GPIO
    if (write_str_to_file("/sys/class/gpio/export", GPIO_NUM) < 0) {
        // 可能已经导出，忽略错误
    }

    // 设置方向为输出
    char path[64];
    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%s/direction", GPIO_NUM);
    if (write_str_to_file(path, "out") < 0) {
        return -1;
    }

    // 打开 value 文件
    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%s/value", GPIO_NUM);
    int fd = open(path, O_WRONLY);
    if (fd < 0) {
        perror("open value");
        return -1;
    }

    printf("Start toggling GPIO %s...\n", GPIO_NUM);

    while (1) {
        if (write(fd, "1", 1) != 1) {
            perror("write 1");
            break;
        }
        delay_us(1000);  // 1ms 高电平

        if (write(fd, "0", 1) != 1) {
            perror("write 0");
            break;
        }
        delay_us(1000);  // 1ms 低电平
    }

    close(fd);

    // 取消导出（可选）
    write_str_to_file("/sys/class/gpio/unexport", GPIO_NUM);

    return 0;
}
```

##### 电池电量计CW2015

```bash
cat /sys/class/power_supply/cw2015-battery/capacity     # 剩余电量 %
cat /sys/class/power_supply/cw2015-battery/status       # Charging/Discharging
#cat /sys/class/power_supply/cw2015-battery/voltage_now  # 电压
```

##### 脑氧采集模块

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <pthread.h>
#include <ctype.h>
#include <errno.h>

#define SERIAL_PORT "/dev/ttyS6"
#define MAX_BUF 256

// 支持的波特率
speed_t get_baudrate(int baud) {
    switch (baud) {
        case 9600: return B9600;
        case 19200: return B19200;
        case 38400: return B38400;
        case 57600: return B57600;
        case 115200: return B115200;
        default: return 0;
    }
}

// 设置串口参数
int set_interface_attribs(int fd, speed_t speed) {
    struct termios tty;
    if (tcgetattr(fd, &tty) != 0) {
        perror("tcgetattr");
        return -1;
    }

    cfmakeraw(&tty);
    cfsetospeed(&tty, speed);
    cfsetispeed(&tty, speed);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8 数据位
    tty.c_cflag &= ~PARENB;                         // 无校验
    tty.c_cflag &= ~CSTOPB;                         // 1 停止位
    tty.c_cflag &= ~CRTSCTS;                        // 无流控
    tty.c_cflag |= CLOCAL | CREAD;                  // 打开接收

    return tcsetattr(fd, TCSANOW, &tty);
}

// 串口读取线程
void* read_thread(void *arg) {
    int fd = *(int *)arg;
    unsigned char buf[128];

    while (1) {
        int n = read(fd, buf, sizeof(buf));
        if (n > 0) {
            printf("接收到: ");
            for (int i = 0; i < n; i++) {
                printf("%02X ", buf[i]);
            }
            printf("\n");
        }
    }
    return NULL;
}

// 将 hex 字符串 "A1 B2 0D" 转为字节流
int hexstr_to_bytes(const char *hexstr, unsigned char *outbuf, int maxlen) {
    int count = 0;
    while (*hexstr && count < maxlen) {
        while (isspace(*hexstr)) hexstr++;
        if (!isxdigit(hexstr[0]) || !isxdigit(hexstr[1])) break;

        char byte_str[3] = { hexstr[0], hexstr[1], '\0' };
        outbuf[count++] = (unsigned char) strtol(byte_str, NULL, 16);
        hexstr += 2;
        while (isspace(*hexstr)) hexstr++;
    }
    return count;
}

// 主函数
int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("用法: %s <baudrate> \"<hex bytes>\"\n", argv[0]);
        printf("示例: %s 115200 \"A1 B2 0D 0A\"\n", argv[0]);
        return 1;
    }

    int baud = atoi(argv[1]);
    speed_t speed = get_baudrate(baud);
    if (speed == 0) {
        fprintf(stderr, "不支持的波特率: %d\n", baud);
        return 1;
    }

    const char *hex_input = argv[2];
    unsigned char tx_buf[MAX_BUF];
    int tx_len = hexstr_to_bytes(hex_input, tx_buf, MAX_BUF);

    if (tx_len == 0) {
        fprintf(stderr, "无效的十六进制输入。\n");
        return 1;
    }

    int fd = open(SERIAL_PORT, O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0) {
        perror("open serial port");
        return 1;
    }

    if (set_interface_attribs(fd, speed) != 0) {
        close(fd);
        return 1;
    }

    pthread_t tid;
    if (pthread_create(&tid, NULL, read_thread, &fd) != 0) {
        perror("pthread_create");
        close(fd);
        return 1;
    }

    if (write(fd, tx_buf, tx_len) != tx_len) {
        perror("write");
        close(fd);
        return 1;
    }

    printf("已发送 %d 字节\n", tx_len);

    pthread_join(tid, NULL);
    close(fd);
    return 0;
}

```

##### leds

```bash
#power led----> user-led0（系统启动，默认开启，用户不用操作）
#开启
echo 255 > /sys/class/leds/user-led0/brightness
#关闭
echo 0 > /sys/class/leds/user-led0/brightness

#ch-l led---->user-led1
#开启
echo 255 > /sys/class/leds/user-led1/brightness
#关闭
echo 0 > /sys/class/leds/user-led1/brightness


#ch-r led---->user-led2
#开启
echo 255 > /sys/class/leds/user-led2/brightness
#关闭
echo 0 > /sys/class/leds/user-led2/brightness
```

##### 按键

```bash
#k4
#放开
cat /sys/class/gpio/gpio122/value
1
#按下
cat /sys/class/gpio/gpio122/value
0

#k6
#放开
cat /sys/class/gpio/gpio123/value
1
#按下
cat /sys/class/gpio/gpio123/value
0

#power_on/off
#放开
cat /sys/class/gpio/gpio142/value
1
#按下
cat /sys/class/gpio/gpio142/value
0
#关机操作可以读取该引脚状态，预定时间执行poweroff命令
```

##### USB

```bash
#检测USB口是否接入
cat /sys/class/android_usb/android*/state
#未接入：DISCONNECTED
#接入：CONFIGURED

#检测USB工作在什么模式
cat /tmp/.usb_config
#U盘：ums
#串口：acm

#串口设备 /dev/ttyGS0

usb_mode.sh ums_en        # 启动 mass storage gadget 并绑定
#####################
usb_mode.sh ums_disable   # 挂载镜像；
usb_mode.sh ums_enable    # 取消挂载镜像；
#挂载和取消挂载现在是热插拔的，拔掉usb会自动挂载，接上会取消挂载，该模式下，用户态可以忽略这两个步骤
usb_mode.sh ums_stop      # 停止ums gadget；该操作最好是在otg未接入的状态下执行
usb_mode.sh acm_en        # 启动虚拟串口 gadget
usb_mode.sh acm_stop      # 停止 acm gadget

```



#### MOH200打印驱动功能

##### 开发环境

* ubuntu20.04
* ubuntu镜像为ubuntu_20.04_moc200_gl

##### 安装依赖

```bash
sudo apt update
sudo apt install qemu-user-static debootstrap #安装容器，构建不同架构的ubuntu根文件系统
sudo qemu-debootstrap --arch=armhf focal ./rootfs http://ports.ubuntu.com/ # 跨架构 rootfs 构建
sudo cp /usr/bin/qemu-arm-static ./rootfs/usr/bin/ # 确保 chroot 环境内能执行ARM 程序
```

##### 切换根路径，进入ARM世界

```bash
sudo chroot ./rootfs /bin/bash #执行该命令后，等同于在arm架构上动作
```

##### 安装cups服务以及hp打印机相关驱动

```bash
# 依赖apt强大的包管理功能，可以省去很多编译安装遇到的问题
apt install -y cups hplip printer-driver-hpcups lsof
```

##### 移植第三方包到该文件系统

```bash
# 将业务程序所需要的依赖拷贝至相关路径
```

##### 系统服务

```bash
# 启动方式迁移（busybox init ---> systemd）;主要是业务程序自启动方式的修改
# 说明：
[Unit]
Description=moc200 start
After=multi-user.target

[Service]
Type=oneshot
ExecStart=/bin/bash /usr/local/bin/moc200_soc.sh
RemainAfterExit=yes

[Install]
WantedBy=multi-user.target

# 其中moc200_soc.sh执行的动作和之前一样，只是更换了启动方式
ss@ss-virtual-machine:~/workspace/rootfs/etc/systemd/system/multi-user.target.wants$ ll moc200.service
lrwxrwxrwx 1 root root 17 11月  6 14:54 moc200.service -> ../moc200.service
# 服务放在/workspace/rootfs/etc/systemd/system/路径下，需要注意的是，要使能服务才可以运行，使能该服务的动作最重要的一步是在/workspace/rootfs/etc/systemd/system/multi-user.target.wants路径下创建了一个链接文件，为避免烧录文件系统第一次无法正常启动程序，这里我们手动在multi-user.target.wants路径下创建moc200.service的链接文件

/home/ss/workspace/rootfs/usr/local/bin/moc200_soc.sh # 开机调用脚本
/home/ss/workspace/rootfs/usr/local/bin/pdf2png_print.sh # 打印脚本
/home/ss/workspace/rootfs/usr/local/bin/usb-mount.sh # usb热插拔挂载脚本
```

##### 其它修改

```bash
# usb热插拔相关（由mdev--->udev）
```

##### 打包文件系统

```bash
sudo tar -cjpf rootfs.tar.bz2 -C rootfs .
```



##### 附录

```bash
# 打印过程中断开USB导致链路正常后无法正常打印问题解决方案
# 在pdf2png_print.sh脚本中加上使能打印机的动作（验证可以解决）
# 示例
#!/bin/bash

cupsenable HP_LaserJet_MFP_M28-M31

# 打印队列状态
if [ "$1" == "status" ]; then
    echo ">> 打印队列状态:"
    lpstat -o
    exit 0
fi

# 开机时间优化
# 0.可以先测试一下，看看那个服务拖慢启动
# 1.把moc200.service的启动时间提前，目前在multi-user.target后，基本上是最后执行的；可以修改这个来提前，这个效果应该也是最显著的
# 示例
[Unit]
After=basic.target
Before=sysinit.target

[Install]
WantedBy=basic.target
# 2.禁用不需要的 systemd 服务；disabled甚至直接删除
# 3.服务并行启动，systemd 默认就支持并行，前提是没有其他的依赖
```



