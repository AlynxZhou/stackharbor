---
title: "在 systemd 服务失败时发送邮件"
layout: post
#comment: true
created: 2026-07-11 11:22:33
categories:
  - 计算机
  - Linux
tags:
  - Linux
  - systemd
---
出门在外的时候，我依赖 headscale 访问家里的设备，但是 headscale 在每次更新后经常会弃用一些配置项，如果它检测到配置文件里仍然有这些配置文件，就会启动失败。为了避免出门在外的时候发现连不上家里，我觉得有必要利用 systemd 进行提醒，以便尽快处理。

传统的方式是通过邮件提醒，当然你也可以使用某些聊天软件的 API。我打算使用一个我平时不用的 Outlook 邮箱发邮件到我常用的邮箱，为了能够用脚本发送邮件，需要安装一些实现了 SMTP 的命令行邮件工具，这里我使用比较简单的 `msmtp`，安装好之后编辑配置文件 `~/.msmtprc`，写入如下配置：

```
# Set default values for all following accounts.
defaults
auth           on
tls            on
tls_trust_file /etc/ssl/certs/ca-certificates.crt
logfile        ~/.msmtp.log

account outlook
host smtp.office365.com
port 587
protocol smtp
from YOUR_EMAIL
user YOUR_EMAIL
auth xoauth2
passwordeval oama access YOUR_EMAIL
tls_starttls on

account default: outlook
```

需要注意的是很多常见的邮件提供商（比如 Gmail 和 Outlook）全都不再支持使用简单密码进行 SMTP 验证，而是强制要求你使用基于 OAuth2 的验证方式，这也是为什么 `passwordeval` 后面跟了很长一串命令的原因。`msmtp` 本身是不支持 OAuth2 的，我们需要安装 `oama-bin` 这个包，通过 `oama` 进行 OAuth2 验证。根据 Arch Wiki，对于 Gmail，需要使用 `auth oauthbearer` 代替 `auth xoauth2`。

安装好 `oama-bin` 之后还不行，如果你直接运行 `oama authorize microsoft YOUR_EMAIL --device` 会失败，因为对于 OAuth2 客户端，Outlook 需要你提供一个 `client_id`。你可以选择去 Outlook 那边申请注册一个你自己的邮件客户端 App 从而得到一个 `client_id`，也可以选择从某个开源邮件客户端的源代码里复制它的，比如我就是用的 Evolution 的。然后编辑 `~/.config/oama/config.yaml`，填写 `microsoft` 下面的 `client_id`。再运行 `oama authorize microsoft YOUR_EMAIL --device`，按照它的提示进行浏览器登录，就可以让 `oama` 正常获取到 OAuth2 的 token 了。

虽然 `msmtp` 可以直接用来发送邮件，但一般更常用的是比如 `mail` 这类工具，因此可以安装 `s-nail` 和 `msmtp-mta`，前者提供 `mail` 命令，后者使 `mail` 命令可以调用 `msmtp` 发邮件。

接下来就可以用比如 `echo "Hello World" | mail -s "Subject" TARGET_EMAIL` 测试能否发送邮件了，通过标准输入传进来的是邮件正文，`-s` 附带的参数是邮件标题。

随后我们准备一个脚本，调用的时候会收集指定的服务的状态作为邮件正文发送，这样我们就不必在 systemd 的 unit 里面写过于复杂的命令。

```bash
#!/bin/bash

mail -S sendwait -s "$2 on $HOSTNAME by systemd" "$1" <<EOF
$(systemctl status --full "$2")

-- Recent 100 Lines of System Journal --

$(journalctl --lines=100 --unit="$2")
EOF
```

把这个脚本保存为 `/usr/bin/systemd-email`。调用的时候第一个参数是要发送到的邮箱地，第二个参数址是 systemd 服务的名字。它会自动收集服务的状态和最近 100 行的服务日志。

然后我们编写一个 systemd 服务，这个服务只做一件事就是调用这个发邮件的脚本，这样我们需要监控哪个服务，就把这个发邮件服务作为对应服务的 `OnFailure` 依赖即可。

```
[Unit]
Description=Email Notification for %i to ANYONE_YOU_WANT

[Service]
Type=oneshot
ExecStart=/usr/bin/systemd-email "TARGET_EMAIL" "%i"
User=YOUR_USER_NAME
Group=systemd-journal
```

把这个文件保存为 `/etc/systemd/system/email-notification@.service`，需要注意的是因为我们把 `msmtp` 和 `oama` 的配置都放在自己的用户家目录，这个 systemd 服务的 `User` 也必须使用我们自己的用户，否则无法读到邮件配置。

随后对于需要监控和提醒的服务，使用 `systemctl edit` 编辑它的服务文件，添加这个发邮件服务的依赖，比如我就是 `systemctl edit headscale`，直接粘贴下面的内容，systemd 会把它合并到原本的服务里：

```
[Unit]
OnFailure=email-notification@%n.service
```

然后执行 `systemctl daemon-reload`，一切正常的话，后续 headscale 启动失败，我就会收到邮件提醒。
