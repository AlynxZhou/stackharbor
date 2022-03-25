---
title: Undefined Script Works!
layout: page
comment: true
created: 2018-04-04 08:15:40
---
# Undefined Script Works!

```plain
「I am the shell of my system.」
「Command is my body, and argument is my blood.」
「I have created over a thousand lines of logs.」
「Unaware of less, nor aware of more.」
「Withstood error to create many scripts, waiting for prompt's arrival.」
「I have no mouse. This is the only bug.」
「My whole life was `Undefined Script Works`!」
```

# 一些个人项目

大概都是些我觉得比较有意思的小玩意。

## Hikaru

生成这个网站的静态生成器。

- 主页：<https://hikaru.alynx.one/>
- 仓库：<https://github.com/AlynxZhou/hikaru/>

## Show Me The Key

在屏幕上显示你按的键，Screenkey 的替代品，采用 libinput 作为后端因此可以同时支持 X11 和 Wayland。

- 主页：<https://showmethekey.alynx.one/>
- 仓库：<https://github.com/AlynxZhou/showmethekey/>

## FlipClock

C 语言实现的一个开源 Fliqlo 替代品（macOS 用户视频里经常出现的翻页时钟屏保）。基于 SDL2。

支持 Linux/Windows/Android 且 **不依赖 Adobe Flash**。可直接设置为 Windows 屏保，绿色免安装。

- 仓库：<https://github.com/AlynxZhou/flipclock/>
- Android 仓库：<https://github.com/AlynxZhou/flipclock-android/>
- Windows 编译好的安装包下载：[点击带有 win 的压缩包](https://github.com/AlynxZhou/flipclock/releases/latest)
- Android 编译好的安装包下载：[点击 apk 文件](https://github.com/AlynxZhou/flipclock-android/releases/latest) 或者去 [酷安网](http://www.coolapk.com/apk/277663) 或 [Google Play Store](https://play.google.com/store/apps/details?id=one.alynx.flipclock)。

## Kouichi 100

“帮助光一天梯上分”小游戏，基本上是个平面版神庙逃亡，完全使用原生 JS 编写。

- 主页：<https://kouichi100.ismyonly.one/>
- 仓库：<https://github.com/AlynxZhou/kouichi100/>

## GNOME Terminal Middle Click Close Tab Patch

一个添加了使用鼠标中键关闭 tab 功能的 GNOME Terminal（上游维护者脾气古怪，觉得鼠标中键关 tab 容易误触并且容易和中键粘贴所以不合 patch，我个人觉得没这么大问题，不过无所谓）。

Arch Linux 用户直接到 [AUR](https://aur.archlinux.org/packages/gnome-terminal-middle-click-close-tab/) 安装。

## GNOME Shell Extension Net Speed

由于 Simple Netspeed 这个扩展一直没有修掉它字符乱闪的 bug（好像作者说是什么他不想让这个玩意总变化宽度所以设置了固定的 width 然后删了就好了但是他不想改，不过说实话我觉得这玩意最多也就变两个字符宽），于是我就自己写了一个，我不太需要它那么多模式也不需要调整字号，所以这个扩展只有一种模式和一种字号。

- 仓库：<https://github.com/AlynxZhou/gnome-shell-extension-net-speed/>
- GNOME Shell 扩展安装页：<https://extensions.gnome.org/extension/4478/net-speed/>
- AUR: <https://aur.archlinux.org/packages/gnome-shell-extension-net-speed/>

## GNOME Shell Extension Fixed IME List

我不知道是哪个脑子抽了的小天才在 GNOME Shell 里加了代码给输入法列表改成了最近优先排序，输入法列表顺序一直变化，于是当我有三个输入法的时候再也没办法不看列表盲切到我想要的那一个。这个扩展通过 hook 掉 GNOME Shell 里面一系列的函数去掉了这个添乱的“特性”，还你一个顺序固定的输入法列表。

- 仓库：<https://github.com/AlynxZhou/gnome-shell-extension-fixed-ime-list/>
- GNOME Shell 扩展安装页：<https://extensions.gnome.org/extension/3663/fixed-ime-list/>
- AUR: <https://aur.archlinux.org/packages/gnome-shell-extension-fixed-ime-list/>

## GNOME Shell Extension Always Show Workspace Thumbnails

更新到 GNOME 40 后如果你刚开机只有一个工作区的话，工作区缩略图现在是不显示的，开发者表示“这是为了方便那些不会用工作区的用户，给他们提供更大的空间”，但我觉得这位小天才显然忽视了我们这些高度依赖工作区的用户，另外我个人觉得既然你把工作区作为自己的主要特性之一，那对于那些不会用工作区的用户应当想办法提示并教会他们使用工作区，而不是迁就他们从而影响到已经在使用这个特性的用户。不过反正我看了一眼代码非常简单，这个扩展 hook 掉那个决定是否显示工作区缩略图的函数，不论什么时候都返回 `true` 就可以了。太过简单以至于我都没有给它打 AUR 的包。

- 仓库：<https://github.com/AlynxZhou/gnome-shell-extension-always-show-workspace-thumbnails/>
- GNOME Shell 扩展安装页：<https://extensions.gnome.org/extension/4156/always-show-workspace-thumbnails/>

## AZTGBot

零依赖 Node.JS Telegram Bot API 框架。

- 主页：<https://tgbot.alynx.one/>
- 仓库：<https://github.com/AlynxZhou/aztgbot/>
- 酒仙：[点这里让酒仙帮你决定晚上吃什么](https://t.me/mangix_bot)

## AZPiano

一个把你的按键映射成钢琴并且能记录下来的 ~~React~~ Web App。更新：已经不是 React 实现的了，因为我发现所有功能我都可以用 Vanilla JS 写，而且 `create-react-app` 引入了大把大把的依赖，不管升级到哪个版本都有可能有风险的依赖。

- 主页：<https://piano.alynx.one/>
- 仓库：<https://github.com/AlynxZhou/azpiano-vanilla/>
- React 版仓库：<https://github.com/AlynxZhou/azpiano-react/>

## Alynx Live Wallpaper

让你选择视频作为壁纸的 Android 应用。

- 主页：<https://livewallpaper.alynx.moe/>
- 仓库：<https://github.com/AlynxZhou/alynx-live-wallpaper/>
- 下载：[酷安网](http://www.coolapk.com/apk/220573) 或 [Google Play Store](https://play.google.com/store/apps/details?id=xyz.alynx.livewallpaper)。
