---
title: Linux 下面常见的代理设置
layout: post
comment: true
createdDate: 2019-02-10 09:28:00
categories:
- 计算机
- 网络
tags:
- 网络
---
通常情况下在 Linux 下面配置好的是 socks5 代理（你懂的），但是这个代理并不能让所有程序自动走它，需要手动的做一些转换。

<!--more-->

# proxychains

proxychains 通过 hook libc 里面网络访问的函数实现劫持流量经过代理，配置时首先编辑 `/etc/proxychains.conf` 添加一行有关自己的代理的配置比如

```
socks5 [IP 地址] [端口号]
```

然后就可以将你需要走代理的程序跟在 `proxychains` 后面比如

```
$ proxychains wget google.com
```

# 环境变量

有些奇葩的语言设计时完全静态链接，不会调用 libc 的运行时库（说的就是那个叫 go 的），因此没办法使用 proxychains 进行 hook。但对于命令行程序，通常有个约定，如果用户设置了 `http_proxy` 和 `https_proxy`，程序就会使用这个作为代理。可以在这两个里设置 socks5 代理，比如 `export http_proxy=socks5://127.0.0.1:1080`，但问题不是所有程序都支持 socks5 代理，所以需要一些方案将 socks5 转换成 http 代理。当然如果你遇到一个 go 程序不读取这两个变量怎么办？当然是在心里问候一下开发者先人。

# privoxy

我个人比较习惯使用 privoxy 做代理转换，当然能实现这个功能的程序非常多，privoxy 是其中一个，安装好之后修改配置文件，添加一行

```
forward-socks5   /               [IP 地址]:[端口号] .
```

然后使用 `systemctl start privoxy` 启动即可。

# 桌面环境

诸如 GNOME 和 KDE Plasma 等都在网络设置里提供了代理选项，按表格填写即可，对于一些内建代理支持的也可以同理比如 Telegram Desktop，本地的 socks5 代理是不需要填写用户名和密码的。对于另外一些同学想要使用 Automatic 的选项的，你需要生成一个 pac 文件，这个文件里记录了哪些网站需要走代理，可以搜索 `你用的 socks5 代理协议的名字` + `pac` 得到更多相关内容，然后在这个选项里填写以 `file://` 开头的 URL，支持的程序就会使用这个 pac。

# 浏览器

通常大家使用插件来调用代理，但如果你上一步设置了桌面环境的代理比如 pac，可以直接走系统代理。

# 端口转发

有时候你的网络很奇葩，连接所有代理服务器都很慢，可以通过一个国内服务器做端口转发，需要在国内服务器执行下列命令。

## 开启流量转发

```
# echo "net.ipv4.ip_forward=1" >> /etc/sysctl.conf
# sysctl -p
```

## 配置转发规则

```
# iptables -t nat -A PREROUTING -p tcp --dport [端口号] -j DNAT --to-destination [代理服务器地址]
# iptables -t nat -A PREROUTING -p udp --dport [端口号] -j DNAT --to-destination [代理服务器地址]
# iptables -t nat -A POSTROUTING -p tcp -d [代理服务器地址] --dport [端口号] -j SNAT --to-source [国内服务器地址]
# iptables -t nat -A POSTROUTING -p tcp -d [代理服务器地址] --dport [端口号] -j SNAT --to-source [国内服务器地址]
```

然后修改你本地的 socks5 配置，IP 改为国内的服务器的 IP。如果国内服务器默认有防火墙（安全组），添加规则放行该端口（TCP + UDP）。

*Alynx Zhou*

**A Coder & Dreamer**
