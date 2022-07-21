---
title: 从 PulseAudio 到 PipeWire
layout: post
#comment: true
created: 2022-06-29T16:11:02
updated: 2022-07-21T10:33:00
categories:
  - 计算机
  - Linux
  - 音频
tags:
  - 录音
  - PulseAudio
  - JACK
  - PipeWire
---
这篇的操作是在之前 [运行在 JACK 上层的 PulseAudio](../Run-PulseAudio-on-Top-of-JACK/) 基础上进行的。

我自己的音频配置比较复杂，虽然 PipeWire 号称能兼容 PulseAudio 和 JACK 的 client 并且在某些发行版成为了默认选项，我还是没很快换掉。因为我想像之前用 PulseAudio 那样把 PipeWire 做成 JACK 的 client，虽然它的文档一直说支持这样，但是看起来两个月前代码才写好。于是我最近尝试了一下。

<!--more-->

首先需要安装 `pipewire`，`pipewire-alsa`，`pipewire-pulse` 和一个 PipeWire Media Session Manager，我用的是新的 WirePlumber 但是不要装 `pipewire-jack` 因为这个是模拟 JACK server 的。然后配置你的 session manager 开启 `alsa.jack-device = true`，然后理论上就能在设置里看到 JACK Sink/Source 了……但是……

首先装 `pipewire-pulse` 会替代 `pulseaudio`，但是按照上篇文章应该是安装了 `pulseaudio-jack` 这个依赖 `pulseaudio` 的包，解决方法是先卸载掉 `pulseaudio-jack`。然后继续安装重新登录应该 PipeWire 已经起来了，按理说这时候启动 JACK 就可以，但是不管我怎么搞都看不到 JACK Sink/Source，所以就备用方案，直接用 PipeWire 替代 JACK 看看，虽然早就可以这么做了，但是之前之前尝试 PipeWire 感觉不是那么稳定，所以就没一直用。

使用 PipeWire 当 JACK 的话要安装 `pipewire-jack`，和前面差不多的问题是 `pipewire-jack` 替代 `jack2` 但是 `jack2-dbus` 依赖 `jack2`，那就先删掉 `jack2-dbus` 再装就好了，然后重新登录，一切正常，Qjackctl 的 Graph 也能正常操作。而且比较有趣的是这样原本使用 PulseAudio 的程序也会在 JACK Graph 里面显示成节点（因为最后都通过 PipeWire），使用 Ardour 录音也没什么问题。其实我的需求还是比较简单的，也不需要什么太低的延迟，只是很多录音的程序都用 JACK 所以才要用。

用了一段时间之后感觉没什么问题，设备之间来回切换也没有卡顿了，驱动我的 2i4 也是完全正常，以后应该就先这样用了。

更新（2022-07-21）：我发现在 Ardour 里面录音还是有问题，具体表现是录超过 1 分钟就会报 xrun，怀疑是这个 bug：<https://gitlab.freedesktop.org/pipewire/pipewire/-/issues/2257>。不过其它 DAW 比如 REAPER 或者 Zrythm 都没问题，我倒是挺想换成 Zrythm 的，但是它还在 Beta 阶段。以及前面板插入耳机似乎设置里检测不到，但是开启一下 pauvcontrol 又能检测到了，总之是一些奇怪的小问题，也许我应该换回去。
