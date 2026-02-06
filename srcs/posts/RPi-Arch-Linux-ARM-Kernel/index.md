---
title: Arch Linux ARM 在树莓派上的内核选择
layout: post
#comment: true
created: 2026-02-06T17:03:00
categories:
  - 计算机
  - 树莓派
tags:
  - 树莓派
---
对于 64 位的树莓派 3 和树莓派 4，安装 Arch Linux ARM 的话有两种内核可以选择，一种是来自博通的 rpi 分支，包含一些树莓派特定的补丁，另一种是来自主线的 mainline 分支，这个和其它系统的内核来源没什么区别。如果你选择安装 64 位的系统，那默认附带的是 mainline 内核，使用上没有什么问题，包括 GPU 驱动也是可用的，但是常见的那些操作树莓派 GPIO 的库和代码都假设你使用的 rpi 内核，会通过 rpi 内核特定的接口操作，因此如果你需要使用 GPIO 但又不想自己从头操作 GPIO 的话就需要使用 rpi 内核。但是默认只有 32 位的 Arch Linux ARM 使用的才是 rpi 内核，在 64 位机器上运行 32 位系统感觉怪怪的。

在我刚拿到 64 位的树莓派的时候就是这么个情况，也许是历史遗留问题，那时候似乎没有运行 rpi 内核的 64 位系统可以选择。但是我最近把吃灰的树莓派 4 拿出来的时候搜索了一下包列表，发现其实已经有 rpi 内核可以用了，这里简单记录一下怎么切换成 rpi 内核。

具体安装只需要安装下面两个包：

```shell
# pacman -S linux-rpi raspberrypi-bootloader
```

Mainline 内核使用 u-boot 作为 bootloader，而 rpi 内核需要使用对应的 bootloader，安装的时候会提示你要移除原本的内核和 u-boot，确认就可以了。然后如果你改过 mainline 内核的启动参数，那需要也对 rpi 内核修改一下，因为两者修改启动参数的方式不一样，rpi 内核和其它的树莓派系统一样使用 `cmdline.txt`。

然后在安装 mainline 内核的系统时你应该已经按照安装说明把 fstab 里面的 `mmcblk0` 换成了 `mmcblk1`，这时候需要换回 rpi 内核使用的 `mmcblk0`。
