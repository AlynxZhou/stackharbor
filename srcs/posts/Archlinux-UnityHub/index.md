---
title: Arch Linux 安装 UnityHub 的临时方案
layout: post
#comment: true
createdTime: 2019-05-10 22:09:00
categories:
- 计算机
- Linux
- Arch Linux
tags:
- UnityHub
---
如果你直接使用 AUR 里 UnityHub 的 PKGBUILD 安装会出一些问题。解决方案也很简单。

首先似乎 PKGBUILD 下载的版本很奇怪，不管你在 Unity 论坛里哪个链接下载的版本其实都是一个，并且和 PKGBUILD 里面的不一样，解决方法就是自己计算一下 md5 然后替换掉 PKGBUILD 里面的 md5sums。

第二个就是 PKGBUILD 里面最后有这几句：

```
# Fix 7z permissions
chmod +x "${pkgdir}/opt/${pkgname}/resources" \
         "${pkgdir}/opt/${pkgname}/resources/app.asar.unpacked" \
         "${pkgdir}/opt/${pkgname}/resources/app.asar.unpacked/external" \
         "${pkgdir}/opt/${pkgname}/resources/app.asar.unpacked/external/7z" \
         "${pkgdir}/opt/${pkgname}/resources/app.asar.unpacked/external/7z/linux64" \
         "${pkgdir}/opt/${pkgname}/resources/app.asar.unpacked/external/7z/linux64/7z"
```

但解压出来实际上没这些文件，然后就会报错，注释掉就好了。

*Alynx Zhou*

**A Coder & Dreamer**
