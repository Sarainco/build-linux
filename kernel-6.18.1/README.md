#### 1.Linux内核获取

```bash
www.kernel.org
```

#### 2.Linux内核初次编译

##### 2.1  安装交叉编译器

```bash
1.https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads

rain@linux:~/workspace/toolchain/arm-gnu-toolchain-13.2.Rel1-x86_64-arm-none-linux-gnueabihf/bin$ ./arm-none-linux-gnueabihf-gcc -v
使用内建 specs。
COLLECT_GCC=./arm-none-linux-gnueabihf-gcc
COLLECT_LTO_WRAPPER=/home/rain/workspace/toolchain/arm-gnu-toolchain-13.2.Rel1-x86_64-arm-none-linux-gnueabihf/bin/../libexec/gcc/arm-none-linux-gnueabihf/13.2.1/lto-wrapper
目标：arm-none-linux-gnueabihf
配置为：/data/jenkins/workspace/GNU-toolchain/arm-13/src/gcc/configure --target=arm-none-linux-gnueabihf --prefix= --with-sysroot=/arm-none-linux-gnueabihf/libc --with-build-sysroot=/data/jenkins/workspace/GNU-toolchain/arm-13/build-arm-none-linux-gnueabihf/install//arm-none-linux-gnueabihf/libc --with-bugurl=https://bugs.linaro.org/ --enable-gnu-indirect-function --enable-shared --disable-libssp --disable-libmudflap --enable-checking=release --enable-languages=c,c++,fortran --with-gmp=/data/jenkins/workspace/GNU-toolchain/arm-13/build-arm-none-linux-gnueabihf/host-tools --with-mpfr=/data/jenkins/workspace/GNU-toolchain/arm-13/build-arm-none-linux-gnueabihf/host-tools --with-mpc=/data/jenkins/workspace/GNU-toolchain/arm-13/build-arm-none-linux-gnueabihf/host-tools --with-isl=/data/jenkins/workspace/GNU-toolchain/arm-13/build-arm-none-linux-gnueabihf/host-tools --with-arch=armv7-a --with-fpu=neon --with-float=hard --with-mode=thumb --with-arch=armv7-a --with-pkgversion='Arm GNU Toolchain 13.2.rel1 (Build arm-13.7)'
线程模型：posix
Supported LTO compression algorithms: zlib
gcc 版本 13.2.1 20231009 (Arm GNU Toolchain 13.2.rel1 (Build arm-13.7))
rain@linux:~/workspace/toolchain/arm-gnu-toolchain-13.2.Rel1-x86_64-arm-none-linux-gnueabihf/bin$
rain@linux:~/workspace/toolchain/arm-gnu-toolchain-13.2.Rel1-x86_64-arm-none-linux-gnueabihf/bin$ pwd
/home/rain/workspace/toolchain/arm-gnu-toolchain-13.2.Rel1-x86_64-arm-none-linux-gnueabihf/bin

export PATH=$PATH:/home/rain/workspace/toolchain/arm-gnu-toolchain-13.2.Rel1-x86_64-arm-none-linux-gnueabihf/bin
```

##### 2.2 搭建编译环境

```bash
sudo apt-get install lzop flex bison libncurses-dev
make ARCH=arm CROSS_COMPILE=arm-none-linux-gnueabihf- distclean
make ARCH=arm CROSS_COMPILE=arm-none-linux-gnueabihf- imx_v6_v7_defconfig
make ARCH=arm CROSS_COMPILE=arm-none-linux-gnueabihf- menuconfig
make ARCH=arm CROSS_COMPILE=arm-none-linux-gnueabihf- all -j8

OBJCOPY arch/arm/boot/zImage
Kernel: arch/arm/boot/zImage is ready

```

#### 3.顶层makefile详解

```bash

```

#### 4.Linux内核启动过程

#### 5.Linux内核移植

```bash
sudo apt-get install tftp-hpa tftpd-hpa xinetd nfs-kernel-server rpcbind



=> print

bootargs=console=ttymxc0,115200 root=/dev/nfs nfsroot=192.168.1.101:/home/rain/workspace/linux/nfs/rootfs,proto=tcp rw ip=192.168.1.17:192.168.1.101:192.168.1.1:255.255.255.0::eth0:off
bootcmd=tftp 80800000 zImage; tftp 83000000 imx6ull-alientek-emmc.dtb; bootz 80800000 - 83000000
gatewayip 192.168.1.1
ipaddr 192.168.1.17
netmask 255.255.255.0
serverip 192.168.1.101
setenv ethaddr b8:ae:1d:01:00:00

setenv bootargs 'console=ttymxc0,115200 root=/dev/nfs nfsroot=192.168.1.101:/home/rain/workspace/linux/nfs/rootfs,proto=tcp rw ip=192.168.1.17:192.168.1.101:192.168.1.1:255.255.255.0::eth0:off'

/home/rain/workspace/linux/nfs *(rw,sync,no_root_squash)

tftp 80800000 zImage   
tftp 83000000 imx6ull-alientek-emmc.dtb 
bootz 80800000 - 83000000 
```

##### 5.1 内核正常启动

```bash
# 该内核串口工作正常，无需额外调试
Starting kernel ...

[    0.000000] Booting Linux on physical CPU 0x0
[    0.000000] Linux version 6.18.1-g2f5f6a894449 (rain@linux) (arm-none-linux-gnueabihf-gcc (Arm GNU Toolchain 13.2.rel1 (Build arm-13.7)) 13.2.1 20231009, GNU ld (Arm GNU Toolchain 13.2.rel1 (Build arm-13.7)) 2.41.0.20231009) #3 SMP Wed Dec 17 14:43:23 CST 2025
[    0.000000] CPU: ARMv7 Processor [410fc075] revision 5 (ARMv7), cr=10c5387d
[    0.000000] CPU: div instructions available: patching division code
[    0.000000] CPU: PIPT / VIPT nonaliasing data cache, VIPT aliasing instruction cache
[    0.000000] OF: fdt: Machine model: Freescale i.MX6 UltraLiteLite 14x14 EVK Board

```

##### 5.2 内核启动后无法通过nfs挂载文件系统

```bash
# nfs会使用到网络，该内核镜像不支持，所以需要调试网口

```

##### 5.3 从emmc加载完整镜像，调试使用tftp加载内核和设备树

```bash
# 1.修改uboot环境变量
# 2.调试内核
setenv bootargs 'console=ttymxc0,115200 root=/dev/mmcblk1p2 rootwait rw' 
setenv bootcmd 'tftp 80800000 zImage; tftp 83000000 imx6ull-14x14-emmc-7-1024x600-c.dtb; bootz 80800000 - 83000000'


root@ATK-IMX6U:~# uname -a
Linux ATK-IMX6U 6.18.1-g2f5f6a894449-dirty #5 SMP Wed Dec 17 17:13:05 CST 2025 armv7l armv7l armv7l GNU/Linux
root@ATK-IMX6U:~#

```

##### 5.5 开发板外设驱动移植适配

