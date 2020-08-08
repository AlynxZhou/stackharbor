---
title: WireGuard 配置与使用
layout: post
comment: true
createdDate: 2019-01-27 10:29:30
updatedDate: 2020-08-08 15:57:00
categories:
- 计算机
- 网络
tags:
- 网络
- VPN
- WireGuard
---
<div class="alert-red">可能有人在 Google 搜索 “WireGuard 原理” 然后发现精选摘要是这篇博文，这给我带来了很多流量。但我还是要摸着良心说这篇博文并没有涉及到什么深层次具体的 WireGuard 原理，只是简单的配置使用记录。我不知道为什么 Google 会依据文章里唯一的一个“原理”就认为这篇文章是讲述 WireGuard 原理的，我不想误人子弟，所以已经删掉了唯一的一处关键词。所以如果你点进来发现走错了，这不是我的责任，建议反馈给 Google 打他们的脸。我个人建议直接访问 WireGuard 的官网或者 Wikipedia 页面来查找有关它的原理的资料。</div>

<div class="alert-red">由于时间久远，记录的事物可能已经改变，作者不能保证此时页面内容完全正确，请不要完全参考。</div>

# 需求

本来想给内网的树莓派做 frp 一类的公网映射以便能够在外面访问 Samba 服务器，但是后来想想似乎把设备暴露在公网不太安全。换个角度想，其实没有必要把树莓派映射出去，只要能够连接进来就好了。所以选用 VPN 建虚拟专用网就好了，这里使用的 WireGuard 将我的各个设备连到一个子网内部。

<!--more-->

WireGuard 和大部分 VPN 类似，是使用一个公网服务器做 Endpoint，内网的数据通过该服务器进行转发，因此子网的各个设备可以没有固定的 IP，只要都能连接公网服务器就好了。

# 安装

由于不同的系统安装方法不一样，建议参照 [WireGuard 官网的安装说明](https://www.wireguard.com/install/) 和各个系统的 Wiki，Android 手机可以搜索下载 WireGuard 的 APK 进行安装。

# 配置

<div class="alert-blue">以下部分内容参考自 <a href="https://wiki.archlinux.org/index.php/WireGuard">WireGuard - ArchWiki</a>。</div>

首先需要生成公钥和私钥，建议将其设置为只有拥有者才能操作，我这里放在 `/etc/wireguard` 这个目录里面，这个目录应该是只有 root 才可以访问的。**服务器和客户端都需要分别进行这一步**。

```
# cd /etc/wireguard
# wg genkey | tee privatekey | wg pubkey > publickey
```

## 服务端

然后需要手写配置文件，这一步对于服务端和客户端不一样，先进行服务端的配置。

首先在 `/etc/wireguard` 目录下面建立 `wg0.conf`（如果你已经有一个了就 `wg1`，依此类推），服务端的配置文件内容看起来像这个样子：

```
[Interface]
Address = 10.0.0.1
ListenPort = 7456
PrivateKey = [服务器的私钥]
PostUp   = iptables -A FORWARD -i %i -j ACCEPT; iptables -A FORWARD -o %i -j ACCEPT; iptables -t nat -A POSTROUTING -o eth0 -j MASQUERADE
PostDown = iptables -D FORWARD -i %i -j ACCEPT; iptables -D FORWARD -o %i -j ACCEPT; iptables -t nat -D POSTROUTING -o eth0 -j MASQUERADE
DNS = 8.8.8.8

[peer]
PublicKey = [客户端 1 的公钥]
AllowedIPs = 10.0.0.2/32

[peer]
PublicKey = [客户端 2 的公钥]
AllowedIPs = 10.0.0.3/32

[peer]
PublicKey = [客户端 3 的公钥]
AllowedIPs = 10.0.0.4/32
```

然后我们依次分析每个字段的内容，首先是 Interface，这个部分指的是 WireGuard 在本机上面的接口，它是以内核模块的形式工作的，`Address` 指本机在子网内的私有 IP 地址，我这里分配了保留段 `10.0.0.0/24` 作为子网网段（看不懂的自己去复习计网），你也可以用 `10.0.0.0/8` 这个网段再进行划分，这个主机地址是你自己随意分配的，只要不冲突就行。`ListenPort` 是监听的服务器 UDP 端口，这个也是自己设置，只要不冲突就行。`PrivateKey` 则是刚刚 **服务端** 生成的 `privatekey` 这个文件的内容。`PostUp` 和 `PostDown` 是配置 iptables 的命令，建议直接复制即可，如果你服务器的公网网卡不是 `eth0` 则需要自己修改。`DNS` 则是解析域名用的 DNS，这里直接使用 `8.8.8.8`。

然后每一个 Peer 字段对应了一个客户端（或其他服务端，WireGuard 实际上是 P2P 的），需要分别将该客户端的 `publickey` 文件的内容贴到 `PublicKey`，以及需要转发到该客户端的流量的 IP 地址范围贴到 `AllowedIPs`，我这里直接给每个客户端设置固定的 IP，因此就写了 `10.0.0.2/32`（看不懂的自己去复习计网），意思是该客户端对应的 IP 就是这个，所有向这个 IP 发送的流量都会被转发到这个客户端。

**每添加一个新的客户端，就要在这个文件里添加一个 Peer 字段**，然后设置 `PublicKey` 和 `AllowedIPs`。

然后在服务器的 `/etc/sysctl.conf` 最后加上一句 `net.ipv4.ip_forward=1`，开启 IPv4 流量转发，然后运行下面的命令加载新设置：

```
# sysctl -p
```

## 客户端

客户端也需要在 `/etc/wireguard` 目录下面建立 `wg0.conf`（如果你已经有一个了就 `wg1`，依此类推），内容看起来是这样：

```
[Interface]
Address = 10.0.0.4
PrivateKey = [客户端的私钥]

[Peer]
PublicKey = [服务端 1 的公钥]
AllowedIPs = 10.0.0.0/24
Endpoint = [服务端 1 的公网 IP]:[服务端 1 的端口]
PersistentKeepalive = 25
```

Interface 字段自然是客户端本机的设置，`Address` 还是给本机分配的 IP，这里要和服务器在同一个子网，并且要和服务器配置文件里允许的 IP 段对上。`PrivateKey` 则是刚刚 **客户端** 生成的 `privatekey` 这个文件的内容。

然后通过添加 Peer 字段来添加服务端，服务端作为这个网络的出口和转发点，首先需要填写对应的 **服务端** 的公钥，然后这里的 `AllowedIPs` 设置的是 **该客户端有哪些 IP 段的流量要通过该服务端进行转发**，由于我只希望内网的流量进行转发，自然设置的是内网的 IP 段 `10.0.0.0/24`（看不懂的你计网挂科了！），然后需要设置 `Endpoint` 以便能正确的在公网上找到服务器（虚拟专用网，连不上网还哪来的虚拟网啊？），格式就是 `服务器公网 IP:你设置的监听端口`。`PersistentKeepalive` 则是用来保持连接的参数，直接复制即可。

**如果你有多个服务器，就在这里添加多个 Peer**，分别设置不同的 `AllowedIPs` 让他们转发，但 **不需要让客户端之间互相添加 Peer**，本来我们要解决的问题就是客户端之间不能互相连接，所有的流量是经由公网的服务端进行转发才可以到达，公网的服务器是各个不固定的客户端的接头地点。所以只需要保证服务端和所有的客户端认识就可以了。

# 启动

在分别配置好客户端和服务端之后就可以启动了，这一步两边都一样。注意把 `wg0` 换成你配置文件的名字。

```
# wg-quick up wg0
```

然后使用 `wg` 命令可以查看状态，由于这些涉及到网络配置，都需要 root 权限。服务端和客户端输出的结果是不一样的，下面这个是我的笔记本输出的结果。

```plain
# wg
interface: wg0
  public key: [不给你看]
  private key: (hidden)
  listening port: 51676

peer: [不给你看]
  endpoint: [不给你看]
  allowed ips: 10.0.0.0/24
  latest handshake: 1 minute, 41 seconds ago
  transfer: 38.36 KiB received, 141.71 KiB sent
  persistent keepalive: every 25 seconds
```

建立成功的话，子网之间是可以使用内网地址互相 ping 的：

```
$ ping 10.0.0.11
PING 10.0.0.11 (10.0.0.11) 56(84) bytes of data.
64 bytes from 10.0.0.11: icmp_seq=1 ttl=64 time=30.1 ms
64 bytes from 10.0.0.11: icmp_seq=2 ttl=64 time=30.4 ms
64 bytes from 10.0.0.11: icmp_seq=3 ttl=64 time=30.5 ms
64 bytes from 10.0.0.11: icmp_seq=4 ttl=64 time=30.4 ms
64 bytes from 10.0.0.11: icmp_seq=5 ttl=64 time=30.5 ms
^C
--- 10.0.0.11 ping statistics ---
5 packets transmitted, 5 received, 0% packet loss, time 11ms
rtt min/avg/max/mdev = 30.133/30.393/30.521/0.209 ms
```

如果你确定没有问题，就可以使用 Systemd 管理连接，首先关闭刚才的连接：

```
# wg-quick down wg0
```

然后启动现成的 Unit：

```
# systemctl enable wg-quick@wg0 --now
```

# 后记

如果你需要在内网之间传递数据，建议选择一个访问较快的服务器，因为数据转发要通过这个服务器。

但如果你想使用 WireGuard 访问一些被河蟹的内容也可以，首先需要一个在海外的服务器，然后设置客户端该服务器的 `AllowedIPs` 为 `0.0.0.0/0`（意思就是所有 IP 都经过这个服务器转发啦！或者你知道该设置某些网段也行！或者避开内网和本地地址！我不建议用这个番茄！所以就写个最蠢的！），就可以了！记得不用时候关掉，不然一些国内 App 也会走海外的！

*Alynx Zhou*

**A Coder & Dreamer**
