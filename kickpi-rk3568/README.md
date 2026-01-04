#### KICKPI-RK3568 SDK开发

##### 1.开发环境搭建

```bash
sudo apt install git bc bison build-essential curl flex
sudo apt install g++-multilib gcc-multilib gnupg gperf libncurses5-dev libncurses5
sudo apt install imagemagick lib32ncurses5-dev lib32readline-dev squashfs-tools 
sudo apt install lib32z1-dev liblz4-tool xsltproc libssl-dev libwxgtk3.0-gtk3-dev libgmp-dev libmpc-dev
sudo apt install libxml2 libxml2-utils schedtool lzop pngcrush rsync 
sudo apt install yasm zip zlib1g-dev  device-tree-compiler 
sudo apt install python2 python3 pip
sudo apt install python-pip gawk openjdk-8-jdk u-boot-tools patchelf expect
sudo pip install pyelftools
sudo ln -s /usr/bin/python2 /usr/bin/python

# 工具链
/home/rain/workspace/toolchain/gcc-arm-10.3-2021.07-x86_64-aarch64-none-linux-gnu/bin
make -j8 CROSS_COMPILE=/home/rain/workspace/toolchain/gcc-arm-10.3-2021.07-x86_64-aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu- ARCH=arm64 rockchip_linux_defconfig
make -j8 CROSS_COMPILE=/home/rain/workspace/toolchain/gcc-arm-10.3-2021.07-x86_64-aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu- ARCH=arm64 rk3568-kickpi-k1-linux.img

# 编译完成
  Image:  resource.img (with rk3568-kickpi-k1-linux.dtb logo.bmp logo_kernel.bmp) is ready
  Image:  boot.img (with Image  resource.img) is ready
  Image:  zboot.img (with Image.lz4  resource.img) is ready

```

