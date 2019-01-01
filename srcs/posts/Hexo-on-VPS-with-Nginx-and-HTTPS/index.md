---
title: 在 VPS 上用 Nginx 和 HTTPS 部署 Hexo 生成的页面
layout: post
comment: true
createdTime: 2018-06-04 16:21:17
categories:
  - 计算机
  - 网络
  - 网站
  - Hexo
tags:
  - Hexo
  - Nginx
---
昨天听说微软要收购 GitHub 了，对于我一个专业微软受害者，虽然它洗白了，但还是有着本能的不信任。同时软粉认软做父的样子我是断然接受不了的（“我微软爸爸天下第一！”“你怎么能不喜欢我微软爸爸呢！”“我微软爸爸什么时候坑过你！”），你自己喜欢就喜欢，我讨厌是我的事情，希望各位嘴下留情，不要给我喷成筛子。

<!--more-->

要说我受害的经历能说好几篇博客，最常见的比如说昨天答辩 PPT 轮到我时候就崩溃了（我可以确定不是我 PPT 的问题，因为关掉所有的 PowerPoint 再打开就没问题了）（“再碰这辣鸡玩意我就是狗！”）。再比如说那个出了名不靠谱的自动更新，你见过更新更新失败回滚回滚失败的嘛？合着我一小时只能对着电脑发呆看你自爆？（当然软粉肯定不承认：“我们更新都成功了！”，这玩意还心诚则灵？反正我被坑了）。倒不是说它收购 GitHub 罪大恶极了，只不过是我自己觉得不舒服就是了，至于普罗大众该干什么干什么，不要管我。

然后我就把我还算有价值的东西——我的网站，搬到了 VPS 上面，搬迁过程相对简单，用 git 做这个同步肯定是不现实的，因为你想像 GitHub 一样可以直接用 `hexo-deployer-git` push 的话，你需要建立的是裸仓库，也就是正常的 `.git` 目录里面的东西，很显然这不是网站内容。看了一圈其他的同步方式，rsync 看起来是最合适的一个了，然后就尝试了一下。

# 用 Rsync 上传 Hexo 生成的网页

首先需要安装 `hexo-deployer-rsync`，另外也要在自己的电脑和 VPS 上面安装 `rsync`，首先进入你的 Hexo 站点目录，然后：

```
$ npm i -s hexo-deployer-rsync
```

根据发行版不同使用不同的命令安装 `rsync`：

Archlinux / Manjaro / Antergos：

```
# pacman -S rsync
```

Debian / Ubuntu / LinuxMint / Deepin：

```
# apt install rsync
```

Red Hat / CentOS / Fedora（如果找不到 `yum` 就换成 `dnf`）：

```
# yum install rsync
```

接下来和很多网上的文章说的不一样，很多人把这里的设置理解成要在 VPS 上面开启一个 rsync 的服务器，实际上不是，rsync可以通过任意一个用户和 ssh 密钥连接到服务器然后复制文件。

首先打开 **你自己的电脑** 上的 `~/.ssh/id_rsa.pub` 获取你自己的公钥（当然你可能用的不是 RSA，自己改），然后复制这个文件，添加到 **服务器** 里面你要使用的用户（不建议 root 敢死队，你平时的用户就可以了）的 `~/.ssh/authorized_keys` 文件里，每个密钥单独粘贴一行。如果你设置过 ssh 免密码登录的话，这一步应该做过了。

然后打开 Hexo 站点的 `_config.yml`，修改 `deploy` 字段，如果没有设置过的话那就改成这样：

```yaml
deploy:
  type: rsync
  host: YOUR VPS IP # 你的服务器的 IP 地址
  user: YOUR USERNAME # 你刚刚复制密钥的那个用户
  root: YOUR DESTINATION # 你想把文件上传到哪里，比如我的是 `~/stackharbor.alynx.xyz/`
  port: 22 # 这是 ssh 默认的端口，如果你修改了，这里也要改
  args: --progress # 额外的 rsync 参数，我这里添加了一个进度条参数，你也可以不设置
  delete: true # 是否删除旧的文件
  verbose: true # 是否同步时显示详细状态
  ignore_errors: false # 忽略错误
```

当然很多人可能已经有一个 git 的部署设置了，那也很简单，设置成一个数组就行了，像这样：

```yaml
deploy:
  - type: git
    repo: # 像这样设置多个 git 仓库，`名称: 地址,分支`，逗号后面没有空格。
      github: git@github.com:XXXXXX/XXXXXX.git,branch
      coding: git@git.coding.net:XXXXXXX/XXXXXXX,coding-pages
    message: Site updated by Hexo at {{ now('YYYY-MM-DD HH:mm:ss') }}.
  - type: rsync
    host: YOUR VPS IP # 你的服务器的 IP 地址
    user: YOUR USERNAME # 你刚刚复制密钥的那个用户
    root: YOUR DESTINATION # 你想把文件上传到哪里，比如我的是 `~/stackharbor.alynx.xyz/`
    port: 22 # 这是 ssh 默认的端口，如果你修改了，这里也要改
    args: --progress # 额外的 rsync 参数，我这里添加了一个进度条参数，你也可以不设置
    delete: true # 是否删除旧的文件
    verbose: true # 是否同步时显示详细状态
    ignore_errors: false # 忽略错误
```

这样就行了，运行 `hexo deploy` 应该就能看到 rsync 的同步状态了。如果你有其他的不使用 Hexo 的网页，可以用命令：

```
$ rsync -azv --delete SOURCE USERNAME@IPADDRESS:DESTINATION
```
<!-- Marked bug: cannot handle `\)` in URL, use `%29` instead. Waiting hexo-renderer-marked update its dependents version. -->
上传到服务器上，如果有 git 仓库最好用 `--exclude=.git` 排除掉，顺便 SOURCE 目录一般不要用 `/` 结尾，原因参见 [这里](https://wiki.archlinux.org/index.php/Rsync_(%E7%AE%80%E4%BD%93%E4%B8%AD%E6%96%87%29#.E6.B3.A8.E6.84.8F.E5.B0.BE.E9.9A.8F.E4.B8.8B.E5.88.92)。

这个时候我还没有把 DNS 从 GitHub Pages 转到我的服务器上，结果生米煮成了熟饭，只能往下继续了。

# 用 Let's Encrypt 生成 SSL 证书

HTTPS 需要有可信机构签发的证书，[Let's Encrypt](https://letsencrypt.org/) 就是一个免费提供这种服务的网站，不过官方推荐的步骤只能对每个域名单独生成证书，比如 `alynx.xyz` 和 `sh.alynx.xyz` 需要两个证书，这里介绍生成通配符证书的办法，比如对 `*.alynx.xyz` 生成一个证书，可以用在所有三级子域名：

## 下载 certbot-auto

`certbot-auto` 是用来验证生成证书的交互命令行工具，似乎一般软件源里的版本都不够高，不能生成通配符证书，这里手动下载新版：

```
$ wget https://dl.eff.org/certbot-auto
$ chmod +x ./certbot-auto
```

## 手动生成证书

生成证书的话需要我们手动操作，命令如下（用你的域名替换掉 `example.com`）：

```
# ./certbot-auto certonly --manual --preferred-challenges=dns --server=https://acme-v02.api.letsencrypt.org/directory -d *.example.com -d example.com
```

这里 `certonly` 表示我们只需要生成证书，我们不需要它帮我们改服务器配置。`--manual` 表示手动配置参数。生成证书需要你证明域名属于你，由于我们是通配符自然不能一个一个页面去放文件验证（默认的验证方法，类似 Google 的页面验证），所以用 `--preferred-challenges=dns` 设置为验证 DNS 记录。`--server=https://acme-v02.api.letsencrypt.org/directory` 表示使用第二版的 api 服务器，默认的第一版是不支持通配符证书的签发的。最后 `-d *.example.com -d example.com` 添加你的域名，需要注意的是只包含 `*.example.com` 的证书是不能用在 `example.com` 上的，所以我们把两个都写上，生成的证书就会同时包含两个域名。

```
Saving debug log to /var/log/letsencrypt/letsencrypt.log
Plugins selected: Authenticator manual, Installer None
Enter email address (used for urgent renewal and security notices) (Enter 'c' to
cancel): YOUR EMAIL
```

这里会让你输入你的邮箱。

```
-------------------------------------------------------------------------------
Please read the Terms of Service at
https://letsencrypt.org/documents/LE-SA-v1.2-November-15-2017.pdf. You must
agree in order to register with the ACME server at
https://acme-v02.api.letsencrypt.org/directory
-------------------------------------------------------------------------------
(A)gree/(C)ancel: A
```

这里输入 A 表示同意条款。

```
-------------------------------------------------------------------------------
Would you be willing to share your email address with the Electronic Frontier
Foundation, a founding partner of the Let's Encrypt project and the non-profit
organization that develops Certbot? We'd like to send you email about EFF and
our work to encrypt the web, protect its users and defend digital rights.
-------------------------------------------------------------------------------
(Y)es/(N)o: N
```

这里问你要不要把邮箱发给 EFF，我不想发就输入了 N。

```
Obtaining a new certificate
Performing the following challenges:
dns-01 challenge for alynx.xyz
dns-01 challenge for alynx.xyz

-------------------------------------------------------------------------------
NOTE: The IP of this machine will be publicly logged as having requested this
certificate. If you're running certbot in manual mode on a machine that is not
your server, please ensure you're okay with that.

Are you OK with your IP being logged?
-------------------------------------------------------------------------------
(Y)es/(N)o: Y
```

这里显示了需要验证的次数，因为我们有两个所以要验证两次，然后为了限制申请次数防止刷证书，这里它会记录你申请证书的 IP，输入 Y。

```
-------------------------------------------------------------------------------
Please deploy a DNS TXT record under the name
_acme-challenge.alynx.xyz with the following value:

YX3oQEVGxLolEVtiFDF4JCKe1tqB7B-9pwis8zuQZkc

Before continuing, verify the record is deployed.
-------------------------------------------------------------------------------
Press Enter to Continue
```

这里是第一次验证，先别急着回车，去你的 DNS 服务商那里添加一条到 `_acme-challenge` 的内容为 `YX3oQEVGxLolEVtiFDF4JCKe1tqB7B-9pwis8zuQZkc`（你的内容不一定和我的一样）的 TXT 记录，然后用 `dig -t txt _acme-challenge.example.com @8.8.8.8`，检查是否更新完成，最好多查几次，防止缓存导致失败。有消息说国内的 DNS 是访问不到的，我这里用的 CloudFlare。

```
-------------------------------------------------------------------------------
Please deploy a DNS TXT record under the name
_acme-challenge.alynx.xyz with the following value:

lKuF76a3GRy7y3M-FVDJ3kif0FQJJwZQUROc6cgG_lk

Before continuing, verify the record is deployed.
-------------------------------------------------------------------------------
Press Enter to Continue
Waiting for verification...
Cleaning up challenges

IMPORTANT NOTES:
 - Congratulations! Your certificate and chain have been saved at:
   /etc/letsencrypt/live/example.com/fullchain.pem
   Your key file has been saved at:
   /etc/letsencrypt/live/example.com/privkey.pem
   Your cert will expire on 2018-09-02. To obtain a new or tweaked
   version of this certificate in the future, simply run certbot-auto
   again. To non-interactively renew *all* of your certificates, run
   "certbot-auto renew"
 - If you like Certbot, please consider supporting our work by:

   Donating to ISRG / Let's Encrypt:   https://letsencrypt.org/donate
   Donating to EFF:                    https://eff.org/donate-le
```

~~又是一次验证，把刚才的记录内容改成新的，~~ 按照评论的指正，这里是可以建立两个同名记录的，所以不需要修改两次，这里可以再添加一条同样名字但内容不同的记录，然后多 `dig` 几次再回车，如果没通过重新运行命令会从记录 IP 那里开始。

需要注意 certbot 似乎会很弱智的用 Nginx 服务的目录的名字作为它认为的你的域名，所以最好在开启 Nginx 之前设置它，它每次都把我的域名当成 `stackharbor.alynx.xyz` 也是把我气个半死。

# 设置 Nginx

这里稍微复杂一点啦，因为我还有另一个页面。

首先我们要安装 Nginx，我的习惯肯定是安装包管理里面的然后用 systemd 管理，我是 Debian9，你们看着来。

## 强制 HTTPS

首先我们要把所有的 HTTP 请求定向到 HTTPS 上，编辑 `/etc/nginx/nginx.conf`，在 `include /etc/nginx/sites-enabled/*;` 前面插入：

```lua
# Forward all HTTP requests to HTTPS.
server {
	listen 80 default_server;
	listen [::]:80 default_server;
	return 301 https://$host$request_uri;
}
```

这段表示所有 80 端口的请求都被 301 永久重定向到 https 的网址上面。

## 配置虚拟主机

然后在 `/etc/nginx/sites-available/` 下面创建一个文件，名字可以随意，我用的是我的域名，然后编辑它：

```lua
# Forward www domain to non-www domain.
server {
	# Must set ssl with a HTTPS request.
	listen 443 ssl http2;
	listen [::]:443 ssl http2;

	ssl_certificate /etc/letsencrypt/live/alynx.xyz/fullchain.pem;
	ssl_certificate_key /etc/letsencrypt/live/alynx.xyz/privkey.pem;
	ssl_dhparam /etc/ssl/certs/dhparam.pem;
	ssl_ciphers 'ECDHE-ECDSA-CHACHA20-POLY1305:ECDHE-RSA-CHACHA20-POLY1305:ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES256-GCM-SHA384:ECDHE-RSA-AES256-GCM-SHA384:DHE-RSA-AES128-GCM-SHA256:DHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-AES128-SHA256:ECDHE-RSA-AES128-SHA256:ECDHE-ECDSA-AES128-SHA:ECDHE-RSA-AES256-SHA384:ECDHE-RSA-AES128-SHA:ECDHE-ECDSA-AES256-SHA384:ECDHE-ECDSA-AES256-SHA:ECDHE-RSA-AES256-SHA:DHE-RSA-AES128-SHA256:DHE-RSA-AES128-SHA:DHE-RSA-AES256-SHA256:DHE-RSA-AES256-SHA:ECDHE-ECDSA-DES-CBC3-SHA:ECDHE-RSA-DES-CBC3-SHA:EDH-RSA-DES-CBC3-SHA:AES128-GCM-SHA256:AES256-GCM-SHA384:AES128-SHA256:AES256-SHA256:AES128-SHA:AES256-SHA:DES-CBC3-SHA:!DSS';
	ssl_prefer_server_ciphers on;
	ssl_protocols TLSv1 TLSv1.1 TLSv1.2;
	ssl_session_cache shared:SSL:50m;
	ssl_session_timeout 1d;
	ssl_stapling on;
	ssl_stapling_verify on;
	ssl_trusted_certificate /etc/letsencrypt/live/alynx.xyz/fullchain.pem;
	add_header Strict-Transport-Security max-age=60;

	server_name www.example.com;
	return 301 https://example.com$request_uri;
}

server {
	# SSL configuration
	listen 443 ssl http2;
	listen [::]:443 ssl http2;

	ssl_certificate /etc/letsencrypt/live/example.com/fullchain.pem;
	ssl_certificate_key /etc/letsencrypt/live/example.com/privkey.pem;
	ssl_dhparam /etc/ssl/certs/dhparam.pem;
	ssl_ciphers 'ECDHE-ECDSA-CHACHA20-POLY1305:ECDHE-RSA-CHACHA20-POLY1305:ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES256-GCM-SHA384:ECDHE-RSA-AES256-GCM-SHA384:DHE-RSA-AES128-GCM-SHA256:DHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-AES128-SHA256:ECDHE-RSA-AES128-SHA256:ECDHE-ECDSA-AES128-SHA:ECDHE-RSA-AES256-SHA384:ECDHE-RSA-AES128-SHA:ECDHE-ECDSA-AES256-SHA384:ECDHE-ECDSA-AES256-SHA:ECDHE-RSA-AES256-SHA:DHE-RSA-AES128-SHA256:DHE-RSA-AES128-SHA:DHE-RSA-AES256-SHA256:DHE-RSA-AES256-SHA:ECDHE-ECDSA-DES-CBC3-SHA:ECDHE-RSA-DES-CBC3-SHA:EDH-RSA-DES-CBC3-SHA:AES128-GCM-SHA256:AES256-GCM-SHA384:AES128-SHA256:AES256-SHA256:AES128-SHA:AES256-SHA:DES-CBC3-SHA:!DSS';
	ssl_prefer_server_ciphers on;
	ssl_protocols TLSv1 TLSv1.1 TLSv1.2;
	ssl_session_cache shared:SSL:50m;
	ssl_session_timeout 1d;
	ssl_stapling on;
	ssl_stapling_verify on;
	ssl_trusted_certificate /etc/letsencrypt/live/example.com/fullchain.pem;
	add_header Strict-Transport-Security max-age=60;

	# Note: You should disable gzip for SSL traffic.
	# See: https://bugs.debian.org/773332
	#
	# Read up on ssl_ciphers to ensure a secure configuration.
	# See: https://bugs.debian.org/765782

	root /home/example/example.com/;

	# Add index.php to the list if you are using PHP
	index index.html index.htm index.nginx-debian.html;
	error_page 404 404.html;
	server_name example.com;

	location / {
		# First attempt to serve request as file, then
		# as directory, then fall back to displaying a 404.
		try_files $uri $uri/ =404;
	}
}
```

首先你看到的 ssl 那一大片都是用来加密的，总之你需要把 example.com 换成你的域名（因为 certbot 生成的证书所在的目录是域名的名字）。然后这里第一个 server 的作用是把 `www.example.com` 重定向到 `example.com`（因为 80 （HTTP）已经被强制转到 443 （HTTPS）了所以一定要写 443 和 ssl 的配置），如果你想反过来，那就把 `return` 那一句加上 `www.` 然后把上下两块的 `server_name` 那一行对调。

`root` 选项设置成你要作为网页目录的路径，比如之前你 Hexo 上传到了家目录下的子目录这里要对应。

接下来创建密钥，Nginx 默认的编码长度太弱，使用 2048 位的：

```
# openssl dhparam -out /etc/ssl/certs/dhparam.pem 2048
```

然后创建软连接使配置生效，并重启 Nginx：

```
# ln -s /etc/nginx/sites-available/example.com /etc/nginx/sites-enabled/
# systemctl restart nginx
```

如果你还有其他的页面，放在另一个目录里，然后再创建一份配置文件，当然这次只要有第二个 server 字段就好了，修改路径，然后连接重启。

# 自动更新证书

安全起见，Let's Encrypt 签发的证书只有 90 天有效期，官方推荐使用自动定期刷新的方式更新证书，只需要建立一个定时任务运行 `/PATH/TO/certbot-auto renew --pre-hook "systemctl stop nginx" --post-hook "systemctl start nginx"`（如果你用的不是 systemd 管理 Nginx 记得换成其它的停止和启动 Nginx 的命令）。很大可能你知道怎么用 crontab 做这件事情，但我使用 systemd-timer 做。

首先建立 `/lib/systemd/system/certbot-renew.service` 这个文件，内容如下：

```ini
[Unit]
Description=Updates cert file via certbot

[Service]
Type=oneshot
ExecStart=/home/alynx/certbot-auto renew --pre-hook "systemctl stop nginx" --post-hook "systemctl start nginx"
```

然后建立对应的 timer `/lib/systemd/system/certbot-renew.timer`，内容如下：

```ini
[Unit]
Description=Updates cert files via certbot every week

[Timer]
OnBootSec=1min
OnUnitActiveSec=1week
Unit=certbot-renew.service

[Install]
WantedBy=multi-user.target
```

这个 timer 会在启动 1 分钟之后运行 certbot-renew.service，同时每周运行一次来确保你的证书会被刷新。

启动它：

```
# systemctl enable certbot-renew.timer --now
```

然后更新你的 DNS 的 A 记录和 AAAA 记录，生效就可以了。

有备无患咯！

更新：我的 VPS 被墙了，算了，噗。

*AlynxZhou*

**A Coder & Dreamer**
