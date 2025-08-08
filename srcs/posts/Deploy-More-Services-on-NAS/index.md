---
title: 在 NAS 上部署更多服务
layout: post
#comment: true
created: 2025-08-08T19:24:38
categories:
  - 计算机
  - Linux
tags:
  - 计算机
  - Linux
  - Caddy
  - PhotoPrism
---
<div class="alert-blue">由于篇幅限制就不写详细的配置教程了，只会记录一下文档里没有的需求和解决方案。</div>

基本上我在 NAS 上部署服务的时候相当克制，虽然我装了一个完整版的 Linux，但其实也就跑了 samba 共享、headscale 和 transmission 下载这几个。我也没有什么出门在外远程看家里的蓝光电影的需求，再说公网网速也很难满足这个需求，所以除非确实需要什么新功能，我会尽量少在 NAS 上运行服务。不过最近我还是发现了一些需求需要部署其它服务来解决。

<!--more-->

# PhotoPrism

一个我早就想解决的需求是按时间线快速查看照片，我发现翻看以前的照片非常有意思，有些已经忘掉的事情看到照片就会想起当时的心情，所以要是能快速的点开过去某个时间的照片就好了。

为了不把自己绑死在某个后期软件上，我没有使用诸如 Lightroom 这类软件内置的照片库管理，而是直接将照片从存储卡里复制到磁盘阵列里。因为不想手动管理重复项，所以我就保持相机的文件夹逻辑。索尼相机虽然在拍照时可以直接按照日期建立文件夹，但是它是前四位序号加后四位月份日期，因此按名称排序并不是时间顺序。即使忽略掉这个问题，也没有办法快速的查看什么时候拍了什么照片，因为文件管理器通常并不能让你隔着文件夹看到里面都有什么照片。再考虑到拍了四年照片之后我已经积攒了相当大的照片库，直接打开一个个目录查看源文件对于磁盘 IO 速度是个相当大的考验。所以是时候部署一个能自动给照片分时分类创建缩略图的服务了。

我选择的方案是 PhotoPrism，当然有好几个类似的项目可选，我就是随便选了一个，没什么特殊理由。PhotoPrism 的官网建议用户以 docker 的方式部署，但我不太喜欢容器化，而且我只有一台服务器也享受不到容器化带来的什么方便，所以我选择通过 AUR 的 `photoprism` 包安装。

官方同时支持 sqlite 和 mysql 两个数据库，但是对于实际的项目使用 mysql 性能要好很多。如果你按照官方的文档使用 docker 应该会要你再部署一个 mariadb 的容器并配置它，但是却没有提如果不用 docker 要怎么办。其实也很简单，总之先装一个 mariadb 配置好并启动，这一部分直接参照 Arch Wiki 就好了。然后我创建了一个叫做 `photoprism` 的数据库和一个 `photoprism` 的数据库用户，再给这个用户设置好密码，给它操作这个数据库的权限，数据库准备工作就算完成了。

使用 docker 的话通常是编辑 compose 文件里面的环境变量来配置 PhotoPrism，不使用 docker 的话则是通过配置文件。AUR 包里面的 PhotoPrism 会加载 `/etc/photoprism/defaults.yml`，里面只有几个路径要设置，当然其实也可以不改路径，毕竟我不打算把照片导入到 PhotoPrism 管理的目录，而是把我的照片目录链接过去，所以这几个目录只会存 PhotoPrism 的配置文件和缓存文件。然后按照官方文档的建议，不要在 `defaults.yml` 里面加更多内容，而是在你设置的 `ConfigPath` 下面创建一个 `options.yml` 在里面进行设置。

`options.yml` 里面的内容和 compose 文件里设置的环境变量的内容其实是等价的，只是写法不同，必须设置的大概也就如下几项，其它的当你启动 PhotoPrism 在设置里修改之后它会自动更新这个文件：

```yaml
AdminPassword: admin
AdminUser: admin
AuthMode: password
DatabaseDriver: mysql
DatabaseName: photoprism
DatabasePassword: your_database_user_password
DatabaseServer: /run/mysqld/mysqld.sock
DatabaseUser: photoprism
DisableTLS: true
HttpPort: 2342
SiteUrl: https://photos.example.com/
```

如果你和我一样 mariadb 和 PhotoPrism 运行在同一台机器上，那可以通过 UNIX socket 指定数据库连接，否则可以使用 IP 和端口。如果你想给别人分享相册，就把 `SiteUrl` 设置成外网的网址。

我不打算使用 PhotoPrism 的导入功能，这个功能会将你的照片复制到它管理的目录并按规则重命名。我打算只让它索引我的照片，所以要将我的照片目录挂载到
 `OriginalsPath` 而不是 `ImportPath`。使用 docker compose 的话会在 compose 文件里指定如何挂载，而我使用 fstab 进行 dir bind mount。以上这些基本上和使用 docker compose 的逻辑一样，只是配置的方式不同。

然后如果修改路径到别处的话，记得保证 `photoprism` 用户对这几个路径是可读写的，因为 systemd 会以 `photoprism` 用户运行 PhotoPrism，`systemctl start photoprism` 之后，内网应该就可以通过 `2342` 端口访问它的前端了。点击资料库 -> 索引，勾选完全重新扫描然后点击开始，应该就可以看到它开始索引目录里的照片并创建缩略图了。

我大概有 24 万张照片，其中一半是 RAW 一半是对应的 JPG，PhotoPrism 运行了一天一夜给我所有的照片建立了索引和缩略图。如果你的 GPU 可以被它用来加速或者你的磁盘阵列速度比我快，应该不需要这么久。

# Jellyfin

我当然是没有出门在外看 NAS 上的视频的需求，但是在家里看的需求还是有的。之前我一直使用文件管理器通过 samba 直接播放，在电脑上当然不成问题，但是不知道是由于无线带宽有限还是 Android 上的 samba 客户端的问题，用手机或者平板看文件体积比较大的电影就加载不动了。Android 版 VLC 虽然有内置的 samba 支持，但是那个文件管理器超级难用啊，没有按日期排序，从一个目录返回上一个目录还不能保持之前的位置。所以我觉得部署一个 Jellyfin 还是有必要的。

Jellyfin 不使用系统的数据库，也不需要修改很多配置文件，大部分都在启动之后的设置向导里设置就可以了。所以安装就是直接在 Arch 官方源里安装 `jellyfin-server` 和 `jellyfin-web`，然后打开 `/etc/jellyfin/jellyfin.env` 看看你需要不需要修改默认的 data 和 cache 目录，然后把存放视频的目录挂载到 data 目录下面就行了。

`systemctl start jellyfin` 之后内网访问 `8096` 端口，按照设置向导设置，之后让它扫描你的视频目录。由于它要生成缩略图，所以也会比较吃资源。至于其它的使用和配置方法应该网上有大把资料，就不啰嗦了。

# Caddy

我一般是不需要在外网访问 NAS 上的服务的，因为我可以通过 tailscale 的 VPN 进行访问。但偶尔我会有通过 web 服务给朋友发文件的需求和给被拍的朋友通过 PhotoPrism 返图的需求，这种情况下就有必要进行 HTTPS 反代了。

要做 HTTPS 反代首先域名我早就有了，北京联通的 IP 虽然不是固定的，但是这个可以通过 DDNS 解决，我自己使用的是 CloudFlare 的 API 和 ddclient，证书的话可以通过 Let's Encrypt 解决，虽然现在流行的 HTTPS 反代服务器都有自动给配置文件里的域名申请证书的功能，但我还是不得不使用 certbot 提供的 DNS-01 验证。因为 HTTP-01 和 TLS-01 验证要求你的 80 或者 443 端口要能被外网访问，但是在国内运营商的网络里你暴露这种常见端口，怕不是觉得自己家水表还是查的不够勤。

至于反代服务器，虽然 NGINX 自己就是个高性能反代服务器，但是它的配置文件还是太过复杂了，我不知道怎么写才能让它反向代理 websocket，PhotoPrism 的文档也说不建议使用 NGINX 反向代理，于是我决定挑一个现代又傻瓜的反代服务器。一开始我本来想尝试 Traefik，但是我看了很久它的文档，感觉全都在写它如何如何支持 docker 服务自动发现，但是我根本没用 docker，也不需要服务发现，我想找如何配置它让它代理我指定的端口，翻来覆去也没看明白。然后我去看 Caddy，发现 Caddy 的文档非常简单易懂，而且核心就是通过简单的手动配置实现各种功能，所以我就用 Caddy 了。

Caddy 和 NGINX 一样既支持文件服务器又支持反向代理，所以我把之前用 NGINX 提供的文件服务也转由 Caddy 负责了。我的需求是在内网我会直接通过 IP 和端口访问对应的服务，不需要 Caddy 转发，然后外网则通过 Caddy 进行 HTTPS 反代。

在官方源里安装 `caddy` 之后打开 `/etc/caddy/Caddyfile`，最下面应该有一行 `import /etc/caddy/conf.d/*`，这一行可以不动，我打算用它实现分离各个不同服务的配置的功能。然后首先实现端口 `2345` 上的文件服务器，非常简单：

```
:2345 {
	file_server browse {
		root /mnt/hdd0/http/fileshare
	}
}
```

只要这么几行就够了，比起写 NGINX 的配置文件还是简单太多了。想要添加 HTTP 的用户密码验证的话就去阅读 Caddy 文档里关于 `basic_auth` 的部分。

然后就是添加外网反向代理的配置，我给 NAS 分配了一个域名，但我并不想让文件服务器作为这个域名的唯一功能，所以我通过 Caddy 给它重定向到一个子目录，这里可以创建一个 `/etc/caddy/conf.d/fileshare`：

```
https://nas_domain.example.com {
	tls path_to_fullchain.pem path_to_privkey.pem

	redir /fileshare /fileshare/
	handle_path /fileshare/* {
		reverse_proxy :2345

	}
}
```

相信这个配置已经简单到不需要我介绍也可以看懂了，至于 PhotoPrism 和 Jellyfin 这样的服务它们不推荐重定向到子目录（会影响 web 资源加载的路径），所以我不得不再分配一个域名给它并且设置 ddclient 和 certbot。然后同样创建一个 `/etc/caddy/conf.d/photoprism`：

```
https://photo_domain.example.com {
	tls path_to_fullchain.pem path_to_privkey.pem

	reverse_proxy :2342
}
```

这看起来比上一个还简单了。启动 Caddy 的话就 `systemctl start caddy`，改了配置文件就 `systemctl reload caddy`。

最后由于我太懒了不想记住每一个 web 服务的端口号，我简单写了个网页列出所有这些服务，这样我在内网访问就可以先打开这个网页再点我想用的。但由于我的 NAS 有好几个网卡，我希望通过万兆直连访问时候就使用万兆网卡的 IP，通过路由器访问的时候就使用板载网卡的 IP，但是 HTML 的链接不能只写端口号，而普通的静态页面显然是不支持按需修改链接的。但是 Caddy 是使用 go 编写的，所以作为 web 服务器的时候支持 go 的模版语法（好吧我并不喜欢 go 的模版语法，但是有总比没有强）。所以可以在 `/etc/caddy/Caddyfile` 里面加入下面一段开启一个支持模版的 web 服务器：

```
http:// {
	root * /mnt/ssd0/http/caddy
	templates
	file_server browse
}
```

然后只要编辑 `/mnt/ssd0/http/caddy/index.html`，在里面用 `{{ .Host }}` 代替本机的 IP 就可以了。
