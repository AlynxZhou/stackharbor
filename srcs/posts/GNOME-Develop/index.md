---
title: GNOME 的修改开发与测试（先备忘，明天完善）
layout: post
comment: true
createdTime: 2019-02-10 22:58:00
categories:
- 计算机
- GNOME
tags:
- GNOME
---
jhbuild 用法，说明。

代理很慢，经常不好用。

colord 的 manpage 依赖 docbook-xsl-ns 但装了也不好用所以要去掉 manpage。

webkitgtk 不负众望是最慢的，

ibus 需要加 `--with-ucd-dir=/usr/share/unicode/` 不然它默认去 `/usr/share/unicode/ucd/` 找文件但 Arch 不是这么打包的。

libgdata 不支持 automake-1.16，需要降级 1.15 来 configure，但是生成的 Makefile 有一句没展开的宏，需要注释掉。

uhttpmock 的来源 URL 是被废弃的仓库，新仓库在 gitlab.com 上，需要修改。

用 wayland 会话似乎既不能 gnome-shell --replace 也不能 gnome-shell --nested （像 weston 小窗口一样，但 gnome-shell 有 lock），所以用 x11 测试。测试时候不要锁屏不要看 lookingGlass，测试完最好重启。

没搞清楚怎么修改 jhbuild 的仓库来源，我都是来回复制文件……
