---
title: OpenWrt 的 dnsmasq 和 odhcpd 可能存在冲突
layout: post
categories:
  - 计算机
  - 网络
tags:
  - OpenWrt
comment: true
created: 2026-08-29 15:25:54
---
最近尝试将家里的网络设备划分成不同的网络，以防止物联网设备偷偷访问我的服务器和工作电脑，这部分我选择询问 Gemini 怎么做，结果还算满意。但是划分之后发现新分出来的网段没有办法联网了，第一反应肯定是最小化排除一下，于是拿了一个电脑单独连接在对应的 LAN 口，结果十分诡异，这个网口只能获取到 IPv6 地址，不能获取 IPv4 地址。继续询问 Gemini 就陷入了典型的 LLM 只会帮倒忙的环节，它开始各种怀疑是哪里配置写错了，但是哪里都没有错。我决定直接看看日志，发现有很可疑的一条：

```
user.notice: dnsmasq: found already running DHCP-server on interface 'br-guest' refusing to start, use 'option force 1' to override
```

原来 OpenWrt 有两个 DHCP 服务，dnsmasq 管理 DHCPv4，对于 DHCPv6 则转给 odhcpd 处理。勾选了 dnsmasq 的“强制”选项以后果然能正常工作了。但是为什么在默认的 lan 网络里就没有问题呢？Gemini 跟我说这是个 bug，也许默认的 lan 网络针对这种情况做过专门的处理，或者是 odhcpd 出了什么 bug。我对此将信将疑，但是当我重启了 odhcpd，然后取消勾选“强制”再重启 dnsmasq，这个问题就消失了。好吧，也许果然是 odhcpd 的 bug。

总之经验教训就是对于自己新建的网络，建议勾选 dnsmasq 的“强制”选项以防万一。毕竟你也不能每次网络连不上的时候再去重启 odhcpd。
