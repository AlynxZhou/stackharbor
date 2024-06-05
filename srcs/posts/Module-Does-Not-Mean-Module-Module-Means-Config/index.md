---
title: Module 的意思不是 Module，Module 的意思是 Config
layout: post
#comment: true
created: 2024-06-05 20:02:37
categories:
  - 计算机
  - 编程
tags:
  - 计算机
  - 编程
  - CMake
---
如果有得选，在开始一个新项目之前我是无论如何也不会选择 CMake 做构建系统的，我不信有任何人觉得 CMake 的语法很友好很容易读，Meson 相比之下更容易读写，而且按我的经验也更好用。但最大的问题是 Meson 出来的很晚，于是有大量的项目在 CMake 已经流行而 Meson 还没出现的时间里把基于 Autotools 的构建系统换成了 CMake（剩下没有更换的那些多数在 Meson 稳定之后更换到了 Meson）。显然把所有 CMake 项目重写成 Meson 是不现实的，最大的困难其实是 CMake 虽然难用，但在“跨平台的构建系统”这一点上已经足够用了，所以对于已有的 CMake 项目，我选择还是凑合用。

实际需要我本人亲手调整 CMake 的项目很少，Ansel 是其中一个，因为当初从 Darktable fork 出来的时候我几乎是完全地整理了一遍它的构建系统，而我今天打开 Element 发现法国老哥在群里 @ 我说是 MSYS2 的 cURL 更新到 8.8.0 导致我们的 CI 构建不了了让我帮他看看，虽然我自己也不是很想写 CMake，但为了避免他一言不合就把依赖复制到项目源码里面我早就跟他说过这种事情我替他处理，所以还是说到做到。

报错就是一堆 undefined reference，我直接去 Google 搜了一下没有搜到什么有用的资料。去 MSYS2 的 issue 搜索 curl 倒是搜到了 <https://github.com/msys2/MINGW-packages/issues/21028>，下面有人说把 `CURL_LIBRARIES` 这个变量换成 `CURL::libcurl` 这个 target 就可以了。我倒是理解这是什么意思，但我想知道这是怎么回事。这中间的过程可以说是曲折离奇。

首先是在这里看到有维护者说这个可能是因为他们把 cURL 从 Autotools 构建换成 CMake 构建导致的，但我其实不是很理解，因为我印象里 CMake 查找 package 是调用 `Find<PackageName>.cmake`，我去看了我们项目本身没有提供这个文件，那应该调用的就是 `/usr/share/cmake/Modules/FindCURL.cmake` 这个由 CMake 本身提供的文件了，但抛开 CMake 这种 Find 模块谜一样的语法不谈，这个文件明确设置了 `CURL_LIBRARIES` 变量，那也就是说这个文件根本没有生效？这时候我去翻了 CMake `find_package()` 的文档，好家伙，这玩意除了有基础和扩展两种语法，还有 Module 和 Config 两种查找模式（内心一万匹草泥马飞奔而过）……如果是 Module 模式，就会用 `FindCURL.cmake` 这个模块，但如果是 Config 模式，就会去调用 `CURLConfig.cmake` 这个配置……而后面这个配置里面只写了 CMake 的 target，没有定义 `CURL_LIBRARIES` 这个变量。

但这样又有两个疑问，首先是为什么会有后面这个 Config 文件呢？毕竟以前一直用的是 Module，我比较了 Arch 的 curl PKGBUILD 和 MSYS2 的 curl PKGBUILD 得出结论：这个文件是 CMake 构建项目的时候自动基于当前所有的 target 生成的，因为 Arch 使用 Autotools 构建的包没有这个文件。但为什么默认会使用这个 Config 呢？按照 CMake 的文档，Config 似乎是 Module 的 fallback，也就是说只有没有 Module 的时候才会使用 Config，我百思不得其解。但总之解决方案很简单，对于 Ansel 自己，我使用了下面的代码：

```cmake
find_package(CURL REQUIRED)
if(TARGET CURL::libcurl)
  set(CURL_LIBRARIES CURL::libcurl)
endif()
```

这样可以保证对不同版本的兼容性，而对于我们自己构建的 exiv2 依赖，我一开始打算直接 `find_package(CURL MODULE REQUIRED)`，毕竟按照文档这样可以强制它忽略 Config 嘛，但结果我推到 CI 上仍然报错。我不是很理解，直到我看了一眼 log 发现它仍然是用的 Config，我当时真的骂人了，我不是写的很清楚要强制 Module 吗？这到底是怎么回事？

当然实际上解决方法是有的，只要使用前一种方法就可以了。但我咽不下这口气，不按照文档说的顺序先尝试 Module 就算了，我强制 Module 了还是给我用 Config 是什么意思？你猜我最后在哪找到了解释，是在 [`FindCURL.cmake` 的文档页面](https://cmake.org/cmake/help/latest/module/FindCURL.html#curl-cmake)：

> If CURL was built using the CMake buildsystem then it provides its own CURLConfig.cmake file for use with the find_package() command's config mode. This module looks for this file and, if found, returns its results with no further action.

我觉得任何一个脑子正常的程序员都该骂做出这个改动的人，首先你改变了默认的加载顺序也就算了，我可以理解成这是你们 CMake 用户的某种隐性约定我新来的我不知道。你用 Config 的时候不遵守 `Find<PackageName>.cmake` Module 约定的导出变量我也忍了。但是请问为什么我在 **显式指定** 了 `MODULE` 关键字的情况下，仍然会被不知不觉的改变行为？这个说明藏在这么小的专题页面里，是生怕用户搞清楚为什么代码和实际行为完全相反吗？写这个逻辑的人有没有意识到自己是在好心办坏事？

这个 `FindCURL.cmake` 是 CMake 官方提供的 module 之一，所以也没必要试图甩锅给其他人。总之我在使用 CMake 的过程中除了迷惑到难以阅读的拼写习惯，反人类且可能混杂不同时期或者不同风味（比如 `find_package()` 就有两种语法）的 API，写起来弯弯绕绕的约定习惯（比如 `Find<PackageName>.cmake` 约定导出 `<PackageName>_LIBRARIES` 然后经常会看到什么 `set(<PackageName>_LIBRARIES <PackageName>_LIBRARY)`）之外，现在又多了个就算你显式指定参数还是会偷偷做出相反行为的体验。我不反对 CMake 确实能用，但和 Meson 的使用体验比起来，写 CMake 确实不能算是什么令人享受的事情。
