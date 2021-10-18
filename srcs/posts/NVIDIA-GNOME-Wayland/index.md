---
title: NVIDIA 驱动和 GNOME 和 Wayland
layout: post
#comment: true
created: 2021-07-30T07:50:39
updated: 2021-09-17T18:51:21
categories:
  - 计算机
  - GNOME
tags:
  - GNOME
  - Wayland
  - NVIDIA
---
由于各种各样的历史原因和近期的变化，我在最近的聊天里发现很多朋友对 NVIDIA 驱动对 Wayland 的支持情况不甚了解，正好我最近在折腾相关的东西，所以打算简单介绍一下我了解的。

<!--more-->

# 常见问题

## 为什么 NVIDIA 的 Wayland 支持这么差？

长话短说，大部分驱动（AMD 的开源和闭源驱动，NVIDIA 的开源驱动 nouveau，Intel 的开源驱动）都同意采用 GBM 作为缓存 API，但是 NVIDIA 像个赌气的小孩一样表示“你们这个不好，我要做个更好的 EGLStreams”所以不支持 GBM，于是所有要兼容 NVIDIA 闭源驱动的混成器都要为它做单独的实现代码。诸如 GNOME 或者 KDE 这种大型项目有足够的人手，同时 NVIDIA 似乎也提供了一定的帮助所以已经有了混成器内部的 EGLStreams 实现，但也有些小型项目并不支持这个玩意。而且 NVIDIA 吹牛皮吹得很大，EGLStreams 哪里“更好”从他们的龟速进展上根本体现不出来。可能是因为挖矿赚得盆满钵满，忘记了显卡的本职工作是显示，NVIDIA 的闭源 Linux 驱动质量一直停留在能用的程度，同时又做各种各样限制导致开源的 nouveau 在较新的显卡上几乎不能用。

NVIDIA 闭源驱动之前一直是稀泥巴糊不上墙的垃圾，因为它在 Wayland 下对使用 XWayland 的程序没有硬件加速支持——也就是说如果你使用 Xorg 会话，程序是有硬件加速的，你使用 Wayland 会话，Wayland 程序也是有硬件加速的，但是在 Wayland 会话下面那些旧的 Xorg 程序就没有硬件加速（听起来就像捡了芝麻丢了西瓜一样）——而仍然有相当一部分旧程序在 Wayland 下依赖 XWayland 运行，所以 NVIDIA 闭源驱动的 Wayland 会话基本可以认为是不能日常使用的。

从版本 470 之后 NVIDIA 闭源驱动有所改善，首先是他们的员工提交的[补丁](https://gitlab.freedesktop.org/xorg/xserver/-/merge_requests/587/diffs)在 XWayland 21.1.1 版本释出了，这个补丁添加了对 XWayland 硬件加速的支持（需要新版本驱动），然后新版本驱动在上周变为稳定版，用户可以安装，所以在拖了至少三年之后最后一块空缺终于被补上了。使用 KDE 和 GNOME 的 NVIDIA 显卡用户理论上就可以获得完整的 Wayland 体验了。

## 现在 NVIDIA 用户的 Wayland 会话可以日常使用了吗？

更新到 470 驱动之后我第一时间体验了一下，~~结果就是很抱歉，还是不行，不过~~ 我遇到的都是一些小问题：

- ~~Firefox 稳定版没法正常工作，但是 Nightly 版本是可以的，原因是 NVIDIA 和 Mesa 的 EGL 实现存在一点小区别，[修复补丁](https://hg.mozilla.org/mozilla-central/rev/c2191ee9cb65)已经在 Nightly 里面了，我们只要等稳定版版本升上去就可以了。~~
- ~~GTK3 程序都能工作，但是 GTK4 的程序（比如 GNOME Extensions）却全都挂了，原因应该和 Firefox 一致，[修复补丁](https://gitlab.gnome.org/GNOME/gtk/-/merge_requests/3726)也已经合并了，只是还没有释出稳定版。~~
- ~~Google Chrome 不能工作，但是比较新的 Chromium 似乎可以，原因推测也是一样的，我懒得查了，同时~~ 所有一直抱着旧版 Electron 不更新的程序应该都有同样的问题。
- GNOME Shell 的缩略图会显示错误，看起来是个[两年的老 bug](https://gitlab.gnome.org/GNOME/mutter/-/issues/528)，最近开发者似乎找到了原因，还没修复。

不过总而言之这些都是能很快修复（废话，大家动作都比 NVIDIA 快，建议老黄把自己的皮衣换成乌龟壳）的问题，不至于被卡很久，至于你的发行版是那种追求“稳定”选择了一个不支持的大版本然后不更新的？哈哈，关我 X 事。

更新（2021-09-17）：Arch 这边 Firefox 更新到了 92.0，所以已经修好了。Google Chrome 现在是 93.0.4577.63，我也能正常打开。GTK4 的 MR 已经释出了稳定版，所以没问题。Atom 在我这用的是 electron9，能用，反而是用 electron12 的 Code - OSS 不能用了……最后那些不知道打包了几百年前的闭源拖拉机（要不把拖去掉变成垃圾？）也就是我指的很明确的 Slack 不能用，加上 `--disable-gpu` 是能用的，但是是他的问题，为什么要我关掉硬件加速消耗 CPU 资源？最后 GNOME Shell 自己的那个 bug……目前没有开发者搞这个，我简单看了一下发现是个我没涉足过的领域，不过总之我觉得是个看起来不爽但不影响你实际使用的问题，所以现在的状态就是 **“又不是不能用”** ！（我可没说后半句啊！上一个说了后半句的珠海小厂现在还活着吗？）

另外我在 openSUSE Tumbleweed 上测试的时候还遇到一个问题啊，它默认安装的是 nouveau，需要手动在 YaST 里面安装 NVIDIA，装完之后似乎两个驱动冲突把我显示器搞黑屏了，于是我就没法直接重启。而且在那之后我显示器神奇的再也不亮了（PS4、Switch 全都点不亮它），拔掉电源放半分钟再插上才好，很头疼啊。

## 我是 GNOME 用户，为什么装了 470 版本 GDM 选单里的 GNOME 还是 Xorg 会话？

这个有几个原因，首先就是 Wayland 需要 Kernel Mode Setting 的支持（这也是个 NVIDIA 拖了好久才补上的特性），所以你的内核参数要有 `nvidia-drm.modeset=1`。其次要看 `/etc/gdm/custom.conf` 里面 `WaylandEnable=false` 是不是被注释掉的（默认应该是）。

最可能的是 udev 规则，GDM 带了一个 `/usr/lib/udev/rules.d/61-gdm.rules` 文件，里面有这么几行：

```
# disable Wayland on Hi1710 chipsets
ATTR{vendor}=="0x19e5", ATTR{device}=="0x1711", RUN+="/usr/lib/gdm-runtime-config set daemon WaylandEnable false"
# disable Wayland when using the proprietary nvidia driver
DRIVER=="nvidia", RUN+="/usr/lib/gdm-runtime-config set daemon WaylandEnable false"
# disable Wayland if modesetting is disabled
IMPORT{cmdline}="nomodeset", RUN+="/usr/lib/gdm-runtime-config set daemon WaylandEnable false"
```

第二行表示如果检测到 NVIDIA 闭源驱动就关闭 Wayland，第三行则是如果没开启 Kernel Mode Setting 也关掉 Wayland。

我猜大部分用户都没有第一行的设备，所以我建议直接 `ln -s /dev/null /etc/udev/rules.d/61-gdm.rules` 屏蔽掉这个文件。

别问我既然 470 驱动都出来了 GNOME 开发者为什么不删掉这个文件，NVIDIA 憋出来 470 驱动总共也就一周多点，还有一堆小问题呢，删了怕不是又有一堆人来骂“怎么又把不稳定的东西搞给我了”。

## 我是笔记本双显卡用户，主用的是核芯显卡，为什么我 GDM 选单里的 GNOME 也是 Xorg 会话？

检查一下你是不是用了 NVIDIA 之前提供的 Xorg 下面的双显卡管理功能（有群友说叫 prime，我不知道是不是叫 prime）（顺便一提这也是 NVIDIA 憋了好几年才憋出来的本来就该有的东西）？如果你使用那个，首先它是 Xorg 下面的支持，其次它会加载 nvidia 模块，自然就会落到上一个问题里面的 udev 规则上。

有群友在群里表示他删了那个规则然后进 Wayland，可以是可以，但我感到很迷惑。这个 prime 功能 NVIDIA 之前只是说了在 Xorg 下面可用，所以我不是很理解又要 prime 又要 Wayland 的可行性，我也不知道 NVIDIA 的按需渲染现在到 Wayland 里面还能不能像之前 Xorg 里面提供的功能一样正常工作。就理论上我给 NVIDIA 掏了钱，这是他的义务给我支持这个功能，但这可是垃圾 NVIDIA 啊！

你问我有没有什么建议？如果你需要这玩意，就按照 GNOME 的规则用 Xorg 就好了，没错虽然我是 Wayland 支持者，但我在这里建议你还是先用着 Xorg。如果你只是想用 Wayland 然后并不是很需要用 NVIDIA 显卡，那就关掉它不让它加载驱动，没错，我在自己的笔记本上用的是 Bumblebee。

## 我是 KDE 用户，你扯这么多 GNOME，跟我说说 KDE 现在有什么问题呗？

大哥，我是 GNOME 用户啊，既然你是 KDE 用户，建议你自己试试然后给我讲讲……

# 没用的观点

## 你看这个 GDM 就是逊啦，我这里卡死根本不能用，大家都推荐换 SDDM 巴拉巴拉……

强调一下我的目的是解决问题，比如群里有人问为什么我双显卡 GDM 里没 Wayland 可选你来这么一句我觉得很不礼貌，当别人就是需要用 GDM 的时候说“大家都推荐换 SDDM”我觉得不管是 GNOME 用户还是 KDE 用户都会觉得不是个好回答。其次只说自己 GDM 卡死完全没法判断问题所在，别人都没这个问题，或许问题就不在 GDM 而在你的配置呢？同理，这个“大家都推荐换 SDDM”里的“大家”是哪里来的呢？我觉得这样的回复对解决问题没什么帮助。

## Xorg 好！Wayland 坏！

请列举出充分而恰当的理由支持你的观点，而不是人云亦云。比方说你可能觉得 Wayland 缺乏你需要的某些功能，这可能是设计理念上的不同，或者发展时间不够所导致的，毕竟 X 协议的历史比 Wayland 长得多，所以请多一点耐心。而至于不讲道理或者二极管或者言论不友善的行为在哪里都是不受欢迎的。

## 我也是 NVIDIA 用户，我玩 XX 游戏卡成幻灯片了！

……信息太少，无法给出合理推断，只能告诉你我游戏玩的可欢乐了！

## 你们 NVIDIA 用户真麻烦，谁叫你们自己掏钱买 N 卡受罪！

你又不知道老子都有什么需求，买 N 卡我是暂时没法用 Wayland，但是不买 N 卡我就要达芬奇受罪 Windows 下面打游戏受罪直播编码受罪跑 hashcat 受罪（存疑）。我做的选择是我权衡之后的结果。再说一遍，解决问题和“换 XX”是完全不一样的态度。

所以就是钱是掏了，骂还是照样骂，而且我都掏钱了，骂得然更有底气了！


