---
title: "使用 Headscale 和 Tailscale 构建虚拟专用网"
layout: post
#comment: true
created: 2024-02-04T19:39:26
categories:
  - 计算机
  - 网络
tags:
  - 网络
  - Tailscale
---
# 需求
很多在家里装了 NAS 的人都有一个相似的需求，那就是出门在外如何访问内网的 NAS 上运行的服务。很多人会选择公网 IP + 端口映射把需要的服务直接暴露到公网上，或者通过公网的 VPS 进行反向代理。但这些我都不放心，首先我的目的只是自己访问，而不是给别人访问，其次对于一些简易的 WebUI，暴露在公网上也容易被无聊的人扫端口并尝试入侵。实际上这个需求更倾向于 VPN（这里指的是它本来的意思也就是虚拟专用网，而不是佛跳墙），我曾经尝试过使用 WireGuard 和公网 VPS 构建一个简单的 VPN，但效果并不好，首先是我的 VPS 并不在国内，作为所有流量的中继实在是太不合适，实际使用起来几乎卡到不能自理，其次是 WireGuard 用作 VPN 服务器的话需要把其它所有 peers 都添加到服务器里，实在是太过麻烦。

第一点我其实没想到什么好的解决办法，能想到最好的也就是利用家里有公网 IP 的特点把 VPN 服务器改到家里。而第二点我差点就想改成自建 OpenVPN 了，但这时我偶然找到一些资料，说不应该手动组建 WireGuard 网络，而是使用一些基于 WireGuard 的工具帮你自动组网。比较之后我决定使用 Tailscale。

Tailscale 能做的并不仅仅是帮你建立一个 VPN 服务器然后自动添加客户端，在此之上它有一些更妙的功能，比如 WireGuard 实际上并不是服务端/客户端架构，peers 之间是对等的，于是 Tailscale 可以尝试通过 NAT 穿透建立点对点的 WireGuard 连接，如果无法穿透才会通过服务端中继（Tailscale 官网有一篇关于如何实现较为可靠的 NAT 穿透的文章，至少我是没怎么看懂），这听起来很适合我的需求并且在实际使用中极大的提升了我的体验。但 Tailscale 本身只是客户端，它们通过销售自己的服务提供服务器供用户连接，客户端是开源的但服务端是闭源的。而我显然更希望自己搭建服务端，幸好有 Headscale 这个开源项目自己实现了一个 Tailscale 服务端，可以自己搭建。但 Headscale 自己的文档非常的简陋，所以我决定写篇博客记录一下具体配置的过程。

# Headscale

首先在公网能访问到的服务器上安装 Headscale，Arch Linux 的官方仓库里已经打包了：

```shell
# pacman -S headscale
```

然后需要修改配置文件 `/etc/headscale/config.yaml`，里面需要修改的只有几处，我这里简单介绍一下：

首先第一个要修改的是 `server_url`，这个就是客户端连接服务器时使用的地址和端口，Headscale 使用的是 HTTP 协议，如果你不想明文在公网上裸奔，那可以在后面添加 HTTPS 证书和密钥使它支持 HTTPS。

同样还需要修改 `listen_addr`，控制 Headscale 监听的网段和端口，这里端口要和上面的一致。

下面其它的控制数据库和 gRPC 都保持默认即可，然后你可以修改想要给子网设备分配 IP 的网段，只要修改 `ip_prefixes` 就可以，要注意的是并不是所有网段都可以用，Tailscale 本身已经限制了一部分，你只能选择这个网段的子网段。我这里注释掉了 IPv6 因为我不需要。

如果你想设置 HTTPS，Headscale 本身支持通过 ACME 帮你自动申请证书，这当然是最好的，但它并不支持通过 DNS 的方式验证域名所有权，也就意味着需要你能够监听 80 或者 443 端口，如果你是公网 IP 的家宽，这基本等于被 ISP 查水表，而如果是 VPS，你也大概率可能在这些端口上运行了其它的 HTTP 服务，所以我没有用这个功能。但它下面还提供了手动指定证书和密钥的选项，你可以使用 certbot 或者 acme.sh 之类的功能帮你处理好证书（和 certbot 搏斗实在是太痛苦了所以我省略了），然后将 `tls_cert_path` 设置为 `fullchain.pem` 所在的路径，`tls_key_path` 设置为 `privkey.pem` 所在的路径就可以。（需要注意 certbot 放置证书的路径只有 root 能读写，而 Headscale 并不是以 root 用户运行的，所以你还需要写 hooks 把文件复制出来并修改权限……）

然后还有一个关于 DNS 的部分需要修改，Tailscale 提供了一个叫做 MagicDNS 的机制，当你连接上这个网络之后，就可以像在家用路由器后面一样通过主机名直接访问对应的设备，或者使用主机名 + 你定义的域名后缀，MagicDNS 会帮你解析到正确的 IP。但这里有一个问题，Headscale 默认的配置会让你运行 Tailscale 的设备将自己的 MagicDNS 服务器设置为 systemd-resolved 对所有域名使用的默认服务器（对没错 Tailscale 客户端上的 DNS 逻辑是被 Headscale 服务端控制的，什么奇怪的脑回路），这其实很不方便，特别是对于国内的一些网站比如 B 站会解析很慢并解析到离你比较远的 CDN 上，所以需要关闭这个功能，只优先对 Tailscale 的域名使用 MagicDNS 服务器。只要将 `dns_config` 下面 `override_local_dns` 设置为 `false` 即可。

然后你还需要修改 `dns_config` 下面 `base_domain` 这一项，这个是 MagicDNS 里内部域名的后缀。

解决了这些之后你就可以启动守护进程：

```shell
# systemctl enable --now headscale
```

Headscale 的进程和相关配置都属于 `headscale` 用户和 `headscale` 组，因此如果你想直接修改相关配置，可以将自己加入 `headscale` 组：

```shell
# gpasswd -a alynx headscale
```

然后你需要创建一个 Headscale 的 user，说是 user 其实更像是 namespace：

```shell
$ headscale users create azvpn
```

上面提到的内部域名的逻辑就是 `主机名.用户名.内部域名后缀`，比如我设置的 `base_domain` 是 `alynx.one`，那 timbersaw 这台主机的内部域名就是 `timbersaw.azvpn.alynx.one`。

后面我们会把设备添加到这个 namespaces，添加的时候自然需要验证权限，一般是 Tailscale 发起请求，Headscale 返回一个链接，打开链接之后是一条指令，你需要将里面的 USERNAME 换成你想要的，然后在 Headscale 所在的机器上运行这个指令。当然如果你不方便 ssh 连到 Headscale 所在的服务器，你也可以创建 preauthkey，直接在 Tailscale 连接时提供即可：

```shell
$ headscale preauthkeys create --user azvpn --reusable --expiration 12h
```

# Tailscale (Linux)

这个同样也在 Arch Linux 的官方仓库，直接安装即可：

```shell
# pacman -S tailscale
```

稍微复杂的一个部分是 DNS，显然 MagicDNS 会修改你的 `/etc/resolv.conf` 设置为自己的 DNS
服务器，但如果你和我的配置相同，那应该这个文件也是由 NetworkManager 管理的。如果你已经理解了 Linux 下面 DNS 解析的逻辑，你应该清楚无论何时都只应该有一个进程管理这个文件。解决方法要么是使用 NetworkManager 的插件来运行 Tailscale 从而只让 NetworkManager 管理 `/etc/resolv.conf`（并没有这样的插件），要么是两者全部放弃自己管理 DNS，交给第三者管理。

无论是 Tailscale 还是 NetworkManager 都能自动检测 systemd-resolved 并配合它工作，所以我们启用这个代替 NetworkManager 管理 `/etc/resolv.conf`，过程很简单也很好理解。

首先把 `/etc/resolv.conf` 链接到 systemd-resolved 的 stub 文件，这个文件的作用只有一个就是把 DNS 服务器设置成 systemd-resolved 运行的 DNS 服务器，这样所有的 DNS 查询就都被传给 systemd-resolved 进行处理：

```shell
# ln -sf ../run/systemd/resolve/stub-resolv.conf /etc/resolv.conf
```

然后启动 systemd-resolved：

```shell
# systemctl enable --now systemd-resolved
```

接下来重启 NetworkManager，当它启动时检测到 `/etc/resolv.conf` 是指向 systemd-resolved 的 stub 文件的软链接，就不会尝试修改该文件而是自动配合 systemd-resolved 工作：

```shell
# systemctl restart NetworkManager
```

然后启动 Tailscale 的守护进程：

```shell
# systemctl enable --now tailscaled
```

接下来就可以尝试连接到 Headscale 服务器：

```shell
# tailscale up --login-server https://YOURSERVER:YOURPORT
```

如果你不想进行上面的手动验证流程，这一步可以直接附加上刚才创建的 preauthkey：

```shell
# tailscale up --login-server https://YOURSERVER:YOURPORT --auth-key YOURPREAUTHKEY
```

此时运行 `ip a`，应该可以看到多了一个叫 `tailscale0` 的网络接口。使用 `resolvectl status` 则可以看到这个接口有自己的 DNS 服务器，并且对 `azvpn.alynx.one` 的域名使用此服务器查询。此时已经可以使用 Tailscale 内网分配的 IP 或者 MagicDNS 提供的域名像在物理路由器后面一样访问内网的各种设备。

# Tailscale (Android)

Tailscale 也有开源的 Android 客户端并且已经上架了 Google Play Store，但你安装之后可能会发现没有自定义服务器的选项，你需要点开并关闭右上角三个点菜单多次，然后菜单里就会多出一项 Change Server，设置成你自建的 Headscale 服务器，然后就可以使用主界面第二个登录选项进行交互登录了。目前似乎 Android 客户端还不支持使用 preauthkey 登录。
