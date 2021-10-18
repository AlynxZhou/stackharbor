---
title: 在 Pixel C 上安装 Lineage OS
layout: post
comment: true
created: 2019-07-24 17:24:00
zone: Asia/Shanghai
categories:
  - 计算机
  - 编程
  - Android
tags:
  - 计算机
  - Android
---
Pixel C 刚出来的时候我就很喜欢，但无奈价格太贵，同时 Android M 不能说是很好的利用平板电脑硬件。由于咕鸽战略失误，现在 Pixel C 已经变成热门洋垃圾了，所以在咸鱼捡了一台玩玩。

官方系统升级只支持到 8.1（真的好意思起名叫 Pixel），Lineage OS 支持也是到 8.1（LOS 版本号就是 15.1，没有 Bug），所有的 9.0 都是 Unofficial 的，不过反正我就拿来看看文档，所以直接刷 LOS 15.1 Official 了（为什么不刷官方？因为我手机不想刷第三方，就拿平板过过瘾）。

<!--more-->

由于刷机时候没拍照片我就不上图了，到手时候已经解锁了，不过既然是亲儿子想必也不会像一些国内厂商一样恶心你。按照 Lineage OS 的说法就是打开开发者选项里 `允许 OEM 解锁` 和 `允许 USB 调试`，按住 `音量下` + `电源` 直到重启进入 bootloader，然后连接电脑，`fastboot devices`，如果检测到了就 `fastboot oem unlock`，然后结束之后重启。（会清除所有数据，并且解锁之后每次开机会在 bootloader 停止 30 秒）。

首先是去 [Lineage OS 官网](https://download.lineageos.org/dragon) 下载给 Pixel C（代号 dragon）的 nightly build ZIP。

然后去 [这个 Google Drive](https://drive.google.com/drive/folders/0By6p5AdQfavBUTZmNWJoaU1iazg) 下载开发者提供的 `vendor.img`。（**务必要下载这个，如果有不止一个就选择最新的，因为 ROM 里不包含 vendor，其他下载的版本又和 ROM 包不匹配。**）

然后去 [TWRP 官网](https://dl.twrp.me/dragon/) 下载给 Pixel C 用的 Recovery。

需要 Google Apps 就去 [Open GApps](https://opengapps.org) 下一个 ZIP，我用的是 `ARM64` -> `8.1` -> `stock`。

如果你需要 Magisk 就自己去下载一个。

把这些都放好，在自己电脑上安装 `adb` 和 `fastboot`，然后打开 `允许 USB 调试`。按住 `音量下` + `电源` 直到重启进入 bootloader，然后 `fastboot flash recovery TWRP-IMG-FILE` 刷入 TWRP，然后再按住 `音量下` + `电源` 直到重启进入 bootloader，选择 Recovery Mode 确认 TWRP 已经刷进去了。

接下来在 TWRP 里面全部 wipe（syste, data, cache, sdcard），然后接上电脑用 `adb push FILE /sdcard/` 把下好的 ROM ZIP、`verdor.img` 和 OpenGApps ZIP 都 push 到 sdcard 里面。

然后选择 Install 刷入 Lineage 的 ZIP。**接下来选择右侧 Install IMG，刷入下好的 `vendor.img`**，然后刷入 OpenGApps 和 Magisk，重启。如果刷入 Magisk 的话会自动重启两次再进入向导。

如果你日后某次更新提示 vendor 不匹配，那就去上面的咕鸽硬盘看看有没有新的刷进去就行了。

接下来就是熟悉的开机向导了。

大致来说作为洋垃圾这个平板还很划算，除了屏幕下端因为接键盘的磁铁的缘故会发黄（通病）、太多磕碰容易花屏（设计缺陷，购买要买少磕碰的），屏幕素质很好尺寸也很大，看文档看 PPT 看五线谱都是绝配（我期末就是用这个看 PPT 复习的超级爽）。但是毕竟是 3 年前的产品，3 GB RAM 不是很够多任务，同时 CPU 性能也不是特别高，甚至窗口特效都会卡（老黄的芯片优化好烂），但是这台机器是 NVIDIA 的芯片，GPU 性能强劲，打开开发者选项里 `禁用 HW 叠加层`，使用 GPU 进行窗口混成特效就不卡了。

游戏性能很弱，大部分游戏都没有对这台机器 GPU 优化，CPU 和内存以及 IO 性能也不高（毕竟三年前），但是玩 FGO 意外的不卡，而且还让我用 **维护送的石头** **单抽** **连续** 出了 **老福** 和 **心心念念的蓝呆**！！！要知道就算把买平板的钱都氪进去也才不到两单，要靠这些石头来双黄蛋简直是白日做梦啊！

除了垃圾 B 站 APP 不支持横屏以外，大部分对我影响都不是很大，所谓生态烂其实对我这种资深 Android 用户也没什么大影响，毕竟我对它分工很明确。日常用起来 Lineage OS 也没什么问题，升级只要傻瓜化点击就 OK 了。


