---
title: GNOME 的修改开发与测试
layout: post
comment: true
createdTime: 2019-02-10 22:58:00
categories:
- 计算机
- GNOME
tags:
- GNOME
---
对于一些 GNOME 比较核心的程序比如 GNOME Shell，调试的时候没办法简单的运行，需要构建一个隔离的环境然后替代系统的 WM。GNOME 项目使用 JHBuild 构建这个环境。我的系统是 Arch Linux，介绍一下中间遇到的一些问题。

<!--more-->

`gitlab.gnome.org` 这个网站国内访问很慢，而且 jhbuild 使用 git clone 有时候不会遵循 git 的代理设置（指 `git config --global http.proxy 'socks5://127.0.0.1:1080' && git config --global https.proxy 'socks5://127.0.0.1:1080'`），经常不好用，我家里的网又不太好。我改 GNOME Shell 代码只用了一小时，但下载和编译环境来测试用了两天一夜。

我调试的是 GNOME Shell，首先按照 [说明页面](https://wiki.gnome.org/action/show/Projects/Jhbuild/Introduction?action=show&redirect=Newcomers%2FBuildSystemComponent%2FJhbuild) 安装 JHBuild，将安装到的 `~/.local/bin` 加入 PATH，然后建立 `~/jhbuild` 目录，运行 `jhbuild build gnome-shell` 会自动下载编译所有需要的依赖（大概 80 个）。

JHBuild 编译下载一些包的时候经常会出问题，一般编译一个包有四个阶段：

- `checkout`：clone 仓库，设置 origin，然后 fetch。
- `configure`：运行 `autogen.sh`，`configure` 或者 `meson`。
- `build`：`make` 或者 `ninja`。
- `install`：就是 `install`，目的地是 JHBuild 的环境。

按了 Ctrl+C 之后会给几个选项，第一个通常是继续当前阶段，第二个是进行下一阶段，第三个是放弃当前的包，第四个是开一个 shell 子程序，第五个是刷新配置文件，对于 JHBuild 不走代理 clone 没速度的时候可以先 Ctrl+C，然后开启 shell，自己手动 clone，然后 Ctrl+D 退出 Shell 回到 JHBuild，这时候会继续让你选择，如果是 checkout 的话选择继续当前阶段，它会发现你同步好了。

如果是某个包需要特殊操作，默认的参数配置不过去的话，建议也是 Ctrl+C 开 shell 手动运行 `./autogen.sh` 之类的，然后选择下一阶段（`configure` 的下一阶段是 `build`），如果继续的话它会再 `configure` 一遍，就覆盖了你的。虽然可以在 JHBuild 配置文件里添加某个包的参数，但是似乎我也搞不清楚 JHBuild 什么时候重新加载配置，经常选了 5 它没变化，把配置文件对应的行删掉它又加上了之前的参数。

反正 JHBuild 搞不定的都手动搞定然后切换下一阶段基本都可以。

`colord` 这个包的 man page 构建时候依赖 `docbook-xsl-ns`，这个包在 AUR 里，但是装上了它也检测不到，所以建议添加 `-Dman=false` 到 `autogen.sh` 的参数，关闭 man page 的生成，反正也用不到（所以 Arch 官方的打包姬怎么打包的啊！）。

`webkitgtk` 不负众望是编译起来最慢的，几年前用 Gentoo 的时候轻松两小时（我当时 Skylake 移动版 Xeon E3-1505M 可是最强的笔记本 CPU），最近有改善了，但还是要半个小时多。

`ibus` 配置的时候需要加 `--with-ucd-dir=/usr/share/unicode/` 不然它默认去 `/usr/share/unicode/ucd/` 找文件但 Arch 打包的路径不是这个。

`libgdata` 不支持 `automake-1.16`，需要降级到 1.15 来 configure，但是生成的 Makefile 有一句没展开的宏（我这里是 4141 行的 `@CODE_COVERAGE_RULES@`）（开发者出来修 bug ！），需要注释掉，然后 `make` 就可以过了，似乎没啥不利影响。

`uhttpmock` 的来源 URL 搞错了，现在指向的旧的是被废弃的仓库，新仓库是 [这个](https://gitlab.com/uhttpmock/uhttpmock/)，需要修改 `~/.config/jhbuildrc` 添加 `branches['uhttpmock'] = ('https://gitlab.com/uhttpmock/uhttpmock.git', 'master')`。

都编译完就可以用 `jhbuild run gnome-shell --replace` 启动你编译的版本了，其他的 GNOME 程序编译的也可以 run 比如 `jhbuild run dconf-editor`。但是用 wayland 会话似乎既不能直接运行 `gnome-shell --replace` 也不能 `gnome-shell --nested` （像 weston 小窗口一样，但 gnome-shell 有 lock），所以我用的 x11 测试。测试时候不要锁屏不要看 lookingGlass，锁屏了回去就会没法输密码解锁，看了 lookingGlass 之后我卡住了……测试完最好重启，不然似乎没啥简单办法回到你系统的程序里。

没搞清楚怎么修改 jhbuild 的仓库来源，也就是说它只会去 GNOME/gnome-shell clone 但没法改成你 fork 的仓库，我只能把改过的文件复制到 `~/jhbuild/checkout` 目录下面对应的仓库里，然后 `jhbuild buildone -n gnome-shell`，`buildone` 的意思是不管其他依赖，只重新编译后面的（用 `build` 会依次检查依赖，很慢的！），`-n` 是就算仓库被改了也不要重新 `checkout`，不然你的文件就被覆盖回去了……

哦对了，GNOME Shell 有个单独的 `gnome-shell-sass` 的仓库存放样式表（CSS/SASS），但看说明这是个只读仓库，是从 `gnome-shell` 里分离出来给别的扩展用的，所以直接修改 `gnome-shell` 里面的就好了。还有一般跟顶栏有关的 GSettings 选项都在 `org.gnome.desktop.interface` 下面，但是这些 schema 对应的 XML 文件都在一个叫 `gsettings-desktop-schemas` 的仓库里，所以需要去那边改。

最后狠狠地吐槽一下砍掉 GNOME Shell 顶栏透明的老哥，为了一点点边缘情况（有人说高对比度颜色壁纸下看不清顶栏文字，但大部分人都看得清并且实在不行换壁纸呗！）就砍掉这个好评如潮的功能。我提了个添加一个 GSettings 选项，让顶栏根据选项决定是透明还是纯黑（壁纸看不清就设成纯黑呗），结果被这个老哥拒绝了，还说添加个选项只会阻止其他人修 bug……大部分人都不觉得这是个 bug 好吗？我不觉得程序识别壁纸颜色改变顶栏会比用户自己调更准确，Android 的变色顶栏过了这么久，在许多壁纸上也不能正确识别颜色，添加一个让用户自己选择的功能才会更方便。但我又不想因为一个奇葩老哥就放弃我这么久的 GNOME 使用习惯，看来还是等等看能折腾出什么名堂吧！

等到 GNOME 3.32 释出稳定版之后我尽量生成一个透明顶栏的 patch 就好了，反正主要的代码都是前人做的，大概 Arch Linux CN 的源愿意接受一个新包。不要和那个奇葩老哥一样说为啥不写扩展，就算是程序员家里养的猫也该知道 GNOME Shell 扩展 API 经常变还经常把整个桌面搞崩，真亏他们好意思提出来！

*Alynx Zhou*

**A Coder & Dreamer**
