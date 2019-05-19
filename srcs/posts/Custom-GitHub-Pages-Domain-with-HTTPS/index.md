---
title: GitHub Pages 自定义域名启用 HTTPS（非 CloudFlare）
layout: post
comment: true
createdTime: 2018-05-02 20:30:48
categories:
  - 计算机
  - 网络
  - 网站
tags:
  - 网站
  - 加密
  - GitHub Pages
---
在 GitHub Pages 支持自定义域名开启 HTTPS 之前，我是使用 CloudFlare 的 CDN 中转来启用 HTTPS 的，缺点也很明显，就是面对国内访问的时候 CloudFlare 的 CDN 简直是逆加速，还不如直连 GitHub 的速度呢。

GitHub 在 2016 年开启了 `*.github.io` 域名的 HTTPS 访问，在今年的 5 月 1 日，终于开启了对自定义域名的 HTTPS 支持。这篇文章将会介绍一下开启的办法和中间容易踩的坑。

<!--more-->

有关为什么要启用 HTTPS 就不多说了，HTTPS 会对传输的内容进行加密，最常见的就是防止无良运营商拦截你的流量添加广告了，也能防止用户接触被劫持的页面而上当受骗，同时不会对浏览造成什么太大的代价。

按照 [GitHub 给出的说明](https://blog.github.com/2018-05-01-github-pages-custom-domains-https/)，似乎我什么都不做就能开启 HTTPS 了，仔细尝试了却发现不是这样。

首先如果你开启了 CloudFlare 的 CDN 和 HTTPS，建议把它们关闭。CloudFlare 的 HTTPS 加密在 *Crypto* 面板里，找到第一项 *SSL*，设置成 `Off`，然后如果你在 *Page Rules* 里面添加了 `Always HTTPS` 之类的选项，可以删除，因为我们可以用 GitHub 提供的 `Enforce HTTPS` 选项进行跳转。然后去到 *DNS* 面板，看到那几朵红色的云朵了吗？它们代表访问时候会首先使用 CloudFlare 的 CDN，考虑到受众，这里可以点击让它变成灰色的 `DNS Only`，从而加快访问速度……

然后如果你是设置 A 记录来跳转到 GitHub Pages 的话，需要修改成新的 IP 地址，GitHub 提供了下面四个 IP，不过我是比较喜欢设置 CNAME 啦：

- 185.199.108.153

- 185.199.109.153

- 185.199.110.153

- 185.199.111.153

如果是 CNAME 的记录就简单啦（虽然 GitHub 文档里推荐的是 ALIAS 或者 ANAME 不过大部分的 DNS 都只有 CNAME 吧），保持你之前 `你的用户名.github.io` 的模式就行啦，比如我是 `AlynxZhou.github.io`。

由于 DNS 多级缓存的存在，DNS 有关的设置需要一定时间来生效，最长大概需要 24 小时，不过一般都是很快的。

然后我们需要去到你开启 Pages 服务的仓库，点击 Settings 面板往下拉，找到 *GitHub Pages* 一节的 *Custom Domain* 选项，重点在这里啦！如果你什么都不做，会发现下面 *Enforce HTTPS* 的选项是灰色的，边上可能还写着说您使用了自定义域名不能开启，难道是虚假宣传？实际上你需要 **先去掉你的自定义域名，然后再添加回来，这里才能开启**，至于你问我为什么，我也不知道。

所以首先我们把这个框里面的自定义域名删掉，然后 save，然后可以访问一下 `https://你的用户名.github.io/`（如果这不是用户主页的话后面要加上仓库名字）测试一下。然后回来再次添加自定义域名，保存。应该可以发现 *Enforce HTTPS* 边上的小字变成了 *正在处理您的 HTTPS 加密证书*（大意），因为这个是 GitHub 和 Let's Encrypt 这个授权验证机构合作推出的服务，证书需要进行处理，**大概一小时之内就可以完成**，然后这次访问 `https://你的自定义域名/`，地址栏里应该就会出现小锁头，表示已经加密，就可以放心的勾选 *Enforce HTTPS* 啦！

*Alynx Zhou*

**A Coder & Dreamer**
