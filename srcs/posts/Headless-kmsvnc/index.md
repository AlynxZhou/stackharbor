---
title: kmsvnc 但是无头
layout: post
#comment: true
created: 2024-03-06T17:38:48
categories:
  - 计算机
  - Linux
tags:
  - 远程桌面
  - VNC
  - kmsvnc
  - NAS
---
由于 Wayland 并不存在像 X11 一样一个单独管理用于最终显示的 framebuffer 的进程，因此远程桌面需要各个桌面做自己的实现。而因为登录界面和用户会话一般是两个会话，远程登录过程中的会话切换就变得非常复杂，虽然 GNOME Remote Desktop 已经有了一个初步实现，但我对那个的逻辑不是很满意，而且他们目前依赖 RDP 的 server rediection，所以并不支持 VNC。

按照当初我做 Show Me The Key 的逻辑，如果一样东西在混成器层面上不好实现，那应该考虑在下一层更统一的层面上实现，[@Jerry Xiao][@Jerry Xiao] 老师做了一个叫做 [kmsvnc](https://github.com/isjerryxiao/kmsvnc/) 的项目，通过 DRM/KMS 获取 framebuffer 并转给 VNC 服务器，虽然它也有一些自己的小问题，但我很喜欢这个方案。

实际上我需要用到远程桌面的场景不多，毕竟 ssh 大部分时候更可靠也更高效，不过有时候需要修改 NAS 上一些桌面设置的时候会用到。但此时就有第一个问题，我的 NAS 是无头（就是没接显示器）的，没有显示器的情况下桌面没有输出，也就获取不到 framebuffer。请教了 [@Jerry Xiao][@Jerry Xiao] 老师之后得知可以通过内核参数实现强制让内核以为我们有个显示器，就可以解决这个问题。

这个目的需要两个内核参数，分别是 `video=` 指定强制开启哪个显卡输出端口，和 `drm.edid_firmware=` 给这个端口指定一个 EDID 从而指定分辨率，正常的显示器会通过端口汇报自己的 EDID，而我们这里没有，所以需要手动指定。

具体这两个参数都支持什么我就不细说了，因为很复杂，建议自己看文档。首先我们得选择一个显卡已经有的输出端口用来做这个，可以用下面的命令：

```shell
$ for p in /sys/class/drm/*/status; do con=${p%/status}; echo -n "${con#*/card?-}: "; cat $p; done
```

它会列出端口和它们的连接状态，比如我这里有三个：

```
DP-1: disconnected
HDMI-A-1: connected
HDMI-A-2: disconnected
```

很神奇的是我的主板上实际只有一个 DP 和一个 HDMI（我的是核芯显卡），经过我的测试主板的 HDMI 输出是 `HDMI-A-2`，至于 `HDMI-A-1` 到底在哪我不清楚，可能是 USB-C，但如果它并没有物理输出，那就更好了，所以我这里选择它。另外我建议不要选择 DP 输出，因为内核内置的 EDID 固件似乎并不支持 DP，导致无法指定分辨率。

然后我们通过 `video="HDMI-A-1:D"` 强制开启这个 HDMI 端口，一般你可能会查到用 `e` 表示强制启用一个端口，不过 `D` 表示强制启用一个数字输出端口，应该用哪个都无所谓。然后我们用 `drm.edid_firmware="HDMI-A-1:edid/1920x1080.bin"` 给这个端口指定一个 EDID 固件，这里我们使用内核内置的 `1920x1080` 分辨率的固件，你也可以选择其它内置的，可以在内核源码 `drivers/gpu/drm/drm_edid_load.c` 文件中找到它们的名字和内容。

然后重新启动系统，再用上面的命令，应该就像我一样有一个 `connected` 的输出了。然后我们启动 kmsvnc：

```
# kmsvnc -p 5901 -b 0.0.0.0 -4 -d /dev/dri/card0
```

再从另一台机器上访问：

```
$ vncviewer YOUR_SERVER_IP:5901
```

应该就能看到登录界面了。不过如果你也用 GDM，你可能会发现登录之后屏幕一直是 GDM 的背景色没有切换到用户会话，这个其实是因为 GDM 会选择另一个空闲的 tty 启动 GNOME，而 [@Jerry Xiao][@Jerry Xiao] 老师表示还没有找到一个普适的方案检测 tty 切换，所以这时候需要你手动 `Ctrl+C` 打断 kmsvnc，然后再重新运行上面的命令，应该就会选择 GNOME 桌面所在的 plane 了。

如果你不是刚开机就启动 kmsvnc，可能会发现它报告找不到 plane 就退出了，这个目测是因为 GDM 检测不到用户输入就会自动停止显示器输出，只要 `systemctl restart gdm` 重新启动一下 GDM 就可以了。

[@Jerry Xiao]: https://github.com/isjerryxiao/
