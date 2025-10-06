---
title: 通过 Intel 无线网卡远程唤醒台式机
layout: post
#comment: true
created: 2025-10-06 12:37:56
categories:
  - 计算机
  - Linux
tags:
  - 计算机
  - Linux
---
有些时候需要开启我的台式机处理一些任务，特别是出门在外的时候，但是台式机里有两块 NVIDIA GPU，如果一直开机也太不节能环保了。所以就想研究一些能远程开机/唤醒的方案。

最简单的方案肯定是 BIOS 里设置来电就开机，然后购买一个智能插座。但是在智能插座到货之前我就要出门，所以决定排列组合一下手头的方案。

BIOS 里通常都会有 Wake on LAN 的支持，这个是网卡收到特定格式的数据包，检查和自己 MAC 地址一致之后就会通过 PCI-E 唤醒电脑。所以一开始我打算尝试这个，反正 Mac mini 待机功耗非常低，我可以一直开着 Mac mini，然后需要的时候 ssh 到 Mac mini 上再给台式机发包唤醒它。

第一个障碍是虽然我的 Mac mini 和 NAS 和台式机之间是有网线连接的，但是 Intel X550 这个万兆网卡是不支持 Wake on LAN 的。所以我不得不再拿一根网线把主板的板载网卡和 Mac mini 连接起来。

参考 [Arch Wiki 的 Wake-on-LAN 页面](https://wiki.archlinux.org/title/Wake-on-LAN)，需要在 BIOS 的高级电源管理里打开由 PCI-E 设备唤醒，但是只打开这个不行，因为这样关机时候网卡还是没有启用的，所以需要打开 UEFI 网络堆栈，让网卡即使是在关机的状态下也可以启用。然后根据网上的说法还需要关闭 BIOS 的快速自检。然后在系统里用 `ethtool -s eth0 wol g` 开启网卡的 Wake on LAN 支持。但是即使以上都打开了，还是不能保证能用。至少我主板上的两个网卡都没法唤醒机器。开机状态下用 netcat 可以看到 Wake on LAN 的 magic packet，关机/睡眠状态下可以看到网卡灯亮着，但是就是没有办法唤醒机器。按道理说 Wake on LAN 只和连接层有关，所以并不需要配置 IP，我也不清楚中间哪里有问题导致不能用，也许是 macOS 或者是什么的限制导致必须指定 IP？但 BIOS 里又没有办法配置 UEFI IPXE 的 IP 地址。

就在我觉得没有希望的时候，@lilydjwg 跟我说其实无线网卡也是可以唤醒电脑的，Intel AX200 确实支持这个功能。只要使用 `iw phy phy0 wowlan enable magic-packet` 启用就可以了。也许需要用 `nmcli con modify CONNECTION 802-11-wireless.wake-on-wlan 0x8` 给 Network Manager 也启用一下。

唤醒的方式和 Wake on LAN 一样，Linux 下面可以使用 `wol`，macOS 下面可以使用 `wakeonlan` 这个脚本。需要注意的是无线和有线一样需要有连接才能收到包，有线只要插着网线就行了，但无线需要保持你的 WiFi 连接，而 UEFI 是没有无线网络支持的，所以 Wake on WLAN 只有睡眠状态能唤醒，不像 Wake on LAN （理论上）支持关机状态开机，但是对我的省电需求来说，休眠到内存也足够了。

因为需要一直保持 WiFi 连接才能用，如果你的网络管理器/无线网卡开启了什么省电模式，它可能会在睡眠一定时间后断掉连接，然后就没办法唤醒了。这自然不是我们希望的，所以可以使用 `nmcli con modify CONNECTION wifi.powersave 2` 关掉 Network Manager 的省电模式，然后给 `iwlwifi` 内核模块设置 `power_save=0` 关掉驱动的省电模式。

使用流程就是 `systemctl suspend` 让电脑休眠。然后用同一个网段的 Mac mini 执行 `wakeonlan -i 目标 IP 地址 目标无线网卡 MAC 地址`，就可以发出 magic packet，台式机收到后就会唤醒了。理论上来说这个和 IP 无关所以可以不指定 IP 地址以发送 magic packet 到广播地址，但是不知道为什么这样唤醒不了。

需要注意的是以上各种依赖的都是 PCI-E 设备可以唤醒系统的功能，因此只有板载的无线网卡或者 PCI-E 的无线网卡可能支持唤醒，USB 无线网卡是不行的。
