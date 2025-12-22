### 目录结构说明

项目根目录主要包含以下内容：

```bash
.
├── configs
├── doc
├── fs-overlay
├── kernel
├── kernel-6.18.1
├── Makefile
├── package
├── README.md
└── rockchip-linux-build
```

### 各目录功能说明

- **build-linux**
   统一的系统构建环境，当前同时支持 **imx6ull、rk3562、rk3568** 三款芯片的平台构建。

- **rockchip-linux-build**
   面向 **RK3568** 平台的独立构建包，采用 Rockchip 官方 SDK 体系，包含：

  ```bash
  .
  ├── buildroot
  │   ├── patches-2018.02
  │   │   └── 0001-update-opencv-mk.patch
  │   ├── rockchip_rk3568_defconfig
  │   ├── rootfs_prepare.sh
  │   └── squashfs
  │       └── 0001-add-squasfs-mutiltiple-define.patch
  ├── kernel
  │   ├── patches-4.19-rk3568
  │   │   ├── 0001-dts_lcd_sensor_config.patch
  │   │   ├── 0002-add-lcd-driver.patch
  │   │   ├── 0003-feat-input-driver-add-drv260x-driver.patch
  │   │   ├── 0004-feat-sys-driver-add-cw2015-driver.patch
  │   │   ├── 0005-feat-input-driver-gpio-input-and-delete-pci-driver.patch
  │   │   ├── 0006-feat-driver-icm42600-add-icm42600-driver.patch
  │   │   ├── 0007-feat-driver-rtc-sd3178-add-rtc-driver-source.patch
  │   │   ├── 0008-feat-add-pwm-beep-driver.patch
  │   │   ├── 0009-feat-i2c-gpio-and-add-opt3001-driver.patch
  │   │   ├── 0010-mipi-lcd-is-ok.patch
  │   │   ├── 0011-sc132gs-sensor-is-ok.patch
  │   │   ├── 0012-rgb-and-mipi-is-ok.patch
  │   │   ├── 0013-drm-is-ok.patch
  │   │   ├── 0014-driver-is-ok.patch
  │   └── rockchip_linux_defconfig
  ├── Makefile
  └── uboot
      ├── patches-2017.09
      │   └── 0001-delete-sd-card-node.patch
      └── rk3568_defconfig
  ```

- **kernel-6.18.1**
   用于移植和验证 **新版本 Linux 内核（6.18.1）** 的构建目录，
   当前仅完成 **imx6ull** 平台的适配，其它 SoC 尚未接入。

- **configs**
   各平台或功能模块的配置文件集合（如 Buildroot、内核配置等）。

- **fs-overlay**
   RootFS 覆盖目录，用于向最终文件系统中追加自定义文件、脚本及配置。

- **kernel**
   存放内核可移植性的驱动。

- **package**
   BusyBox 及第三方软件包的构建描述与定制内容。

- **staging**
   解压后的源码统一存放目录，供构建系统使用。

- **doc**
   项目相关文档与说明资料。

- **Makefile**
   项目顶层构建入口，用于统一调度各模块的编译流程。

- **README.md**
   项目总体说明文档。

### RootFS 构建说明

除内核与平台相关构建外，其余部分主要基于 **BusyBox + 第三方软件包** 方式构建 RootFS，并通过 `fs-overlay` 进行定制化补充。

* 注：该项目包不包含任何源码，为纯净构建包，源码包在其他路径，如需了解包含哪些工具包，可以通过顶层Makefile获知