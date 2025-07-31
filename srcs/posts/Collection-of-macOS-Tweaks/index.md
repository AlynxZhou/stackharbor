---
title: macOS 微调技巧合集
layout: post
#comment: true
created: 2025-07-31T16:33:55
categories:
  - 计算机
  - macOS
tags:
  - 计算机
  - macOS
---
以下记录了一些常见或者不常见的 macOS 微调技巧，其中有些是因为 macOS 的默认设置过于难用，然后又很难想到用什么关键词才能搜索到的选项，有些则是作为开发环境和 Linux 不一样的地方，可以进行调整。

# 关闭拖拽到屏幕边缘并等待切换工作区

作为一个桌面程序员，我认为最令用户恼火的使用体验就是按住并等待了，你不知道要等待多久才会触发，往往你以为已经触发了结果还没有，极大地降低了日常操作的效率。但是似乎苹果的设计师并没有意识到这种设计在电脑上有多难用，并且在多个地方使用了按住并等待触发的逻辑。其中甚至包含互相冲突的功能：比如 macOS 15 新增的拖动窗口到屏幕边缘分屏，这个和拖动窗口到屏幕边缘切换工作区完全冲突。你必须小心翼翼的拖动窗口到边缘，但是又不能完全碰到边缘，才可以完美的触发分屏而不是切换工作区。这在日常使用中几乎是不可能的，考虑到分屏更常用并且有多种方法切换工作区，关掉这个愚蠢的功能才是合理的。但是苹果并没有提供直接关掉这个功能的选项，只能通过把触发的等待时间加大到几乎不可能误触发来近似实现了。

```shell
$ defaults write com.apple.dock workspaces-edge-delay -float 60 && killall Dock
```

这个指令会将等待时间加大到 60 秒，我相信没有人会在边缘按住鼠标 60 秒吧。

# 减小右上角托盘图标间距

macOS 右上角托盘图标之间的间距简直大到离谱，特别是在有刘海的屏幕上，本来就不宽裕的长度根本容纳不下几个图标。很多用户被迫购买一些付费的软件来折叠显示托盘图标，实际上这个间距是可以调小的，只是一下子很难想到。

```shell
$ defaults -currentHost write -globalDomain NSStatusItemSpacing -int 5
```

这个会将状态栏图标的间隔缩小为 5。

```
$ defaults -currentHost write -globalDomain NSStatusItemSelectionPadding -int 3
```

这个会将状态栏图标点击和悬浮的显示特效宽度缩小为 3。

也许需要注销并重新登录才会生效。在这个间距下只要再关掉一些不需要一直显示的图标，应该就可以显示所有日常需要的托盘图标了。

# 兼容使用 GNU 版本命令行工具的脚本

绝大多数 Linux 发行版附带的命令行工具都是 GNU 版本，而 macOS 附带的则是 BSD 版本，这两者各自包含一些不同的命令行选项。你可以通过 Homebrew 安装各种 GNU 版本的工具，但 Homebrew 默认会给这些命令加上 `g` 前缀以和系统自带的 BSD 版本区分开，并且会把不带 `g` 的文件安装到单独的目录里而不是加入到 `PATH` 里。

如果只是顺应个人的使用习惯，那可以通过以下的方式设置 `PATH` 来默认使用 GNU 版本的工具：

```shell
$ export PATH="/opt/homebrew/opt/coreutils/libexec/gnubin:${PATH}"
```

但是对于另一些命令行选项区别更大而又常用于脚本的工具（比如 `sed`，`find`，`grep`），直接设置 `PATH` 虽然可以兼容一些在 Linux 下开发的脚本，但是可能会不支持在 macOS 下开发的脚本，因为作者可能使用的是 BSD 版本的工具。我的解决方案是设置一个定义 `PATH` 的 alias：

```shell
$ alias gnubin="PATH=\"\
/opt/homebrew/opt/coreutils/libexec/gnubin:\
/opt/homebrew/opt/findutils/libexec/gnubin:\
/opt/homebrew/opt/grep/libexec/gnubin:\
/opt/homebrew/opt/gnu-sed/libexec/gnubin:\
/opt/homebrew/opt/gnu-tar/libexec/gnubin:\
${PATH}\
\""
```

这样当执行 GNU 版本的命令或者脚本时，只要在前面加上 `gnubin` 再执行即可。

# 跨设备接收文件时使用较新的 rsync

macOS 自带了一个版本比较旧的 rsync，你可以使用 Homebrew 自行安装比较新的 rsync，但是当你从其他 Linux 机器通过 rsync 向 macOS 机器传文件时可能会报错，提示你使用的命令行选项不支持。这是因为 rsync 默认通过 SSH 进行跨设备文件传输，而 SSH 默认不会读取用户设置的 `PATH` 环境变量，直接去 `/usr/bin` 下面调用使用的是 macOS 自带的老版本 rsync，也就无法支持比较新的版本才有的选项了。

解决方法是先编辑 `/etc/ssh/sshd_config`，设置 `PermitUserEnvironment PATH` 让它读取用户设置的 `PATH` 环境变量。

然后编辑 `~/.ssh/environment` 给 `PATH` 前面添加 Homebrew 的目录：

```
PATH=/opt/homebrew/bin:/usr/bin:/bin:/usr/sbin:/sbin
```

重新启动 SSH 服务，应该就可以调用 Homebrew 安装的 rsync 了。

# Open Build Service

这一部分也许只有我自己工作会用到，或者你也使用 Open Build Service，需要在 macOS 上运行 `osc`。

Homebrew 安装的 `osc` 在 macOS 里读不到系统的证书，所以即使将 SUSE 的证书导入到系统，访问 `api.suse.de` 时候也会报错。

解决方法是在 `~/.config/osc/oscrc` 里给 `[https://api.suse.de]` 设置 `cafile=/Users/alynx/openvpn/SUSE/SUSE_Trust_Root.crt` 让它能自己找到 SUSE 证书。

`osc vc` 调用的是 `obs-build` 里面的 `vc` 脚本，但是由于 macOS 默认 APFS 设置了不区分大小写，而 `obs-build` 源码里包含 `Build` 和 `PBuild` 目录，以及 `build` 和 `pbuild` 脚本，macOS 会直接报文件名冲突，根本没办法解包 `obs-build` 的源码，也就没办法打包了。虽然重新格式化 APFS 可以设置区分大小写，但一些屎山软件比如 Adobe 家的垃圾只支持不区分大小写的文件系统，所以这么做得不偿失。2025 年还不支持区分大小写简直是犯罪啊。

解决方法是直接去 <https://github.com/openSUSE/obs-build/raw/refs/heads/master/vc> 下载这个脚本到 `~/bin/osc-vc` 之类的地方，然后在 `~/.config/osc/oscrc` 给 `[general]` 设置 `vc-cmd=/Users/alynx/bin/osc-vc` 让它找到这个脚本就行了。

默认 `osc` 会去 `/usr/lib/obs/service` 下面寻找各种 `obs-service` 插件，但自然你在 macOS 下面是不能安装到这个目录的，而且这些插件都没有打包成 Homebrew 的 Formula。我自己创建了一个 Tap 在 <https://github.com/AlynxZhou/homebrew-openbuildservice/>，里面的 `osc` 将插件目录修改成了 `${HOMEBREW_PREFIX}/lib/obs/service`，并且将其他的插件的安装位置也修改了，所以建议你安装我的版本而不是 Homebrew core 里面的。不过这些插件大部分都依赖 `obs-build` 里面的各种脚本才能运行，所以直到 `obs-build` 同意修改目录名字来兼容不区分大小写的文件系统之前，都只能通过 `osc commit --noservice` 的方式凑合用了。
