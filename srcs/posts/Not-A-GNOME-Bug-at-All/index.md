---
title: 都不能算是 GNOME 的 Bug
layout: post
#comment: true
created: 2023-05-08T18:52:40
categories:
  - 计算机
  - GNOME
tags:
  - GNOME
  - NVIDIA
---
Arch Linux 的官方仓库里终于有 GNOME 44 了，今天更新了一下系统，在思考出怎么解决 DaVinci Resolve 一定要去加载 onetbb 里面 intel 的 OpenCL 实现之前，我遇到了一个更奇怪的问题：所有的 XWayland 程序都显示不出来窗口，程序启动了，没有报错，但是点不到。

忘了我当时在查什么反正看了一下 `journalctl -f` 发现一直刷一个 `mutter-x11-frames` core dump 的 log，我想起来 mutter 44 应该是把 X11 程序的 decoration 挪到单独的 client 里面实现了，所以也许是 mutter 的问题，不过我还是尝试用 gdb 看了一下 backtrace：

```
Thread 1 "mutter-x11-fram" received signal SIGSEGV, Segmentation fault.
___pthread_mutex_lock (mutex=0x123) at pthread_mutex_lock.c:80
Downloading source file /usr/src/debug/glibc/glibc/nptl/pthread_mutex_lock.c
80        unsigned int type = PTHREAD_MUTEX_TYPE_ELISION (mutex);                                                                                                                                                                                             
(gdb) bt
#0  ___pthread_mutex_lock (mutex=0x123) at pthread_mutex_lock.c:80
#1  0x00007ffff685aaf6 in wl_proxy_create_wrapper (proxy=proxy@entry=0x55555558e510) at ../wayland-1.22.0/src/wayland-client.c:2446
#2  0x00007ffff2ad337c in getServerProtocolsInfo (protocols=0x7fffffffdc70, nativeDpy=0x55555558e510) at ../egl-wayland/src/wayland-egldisplay.c:464
#3  wlEglGetPlatformDisplayExport (data=0x5555555ae000, platform=<optimized out>, nativeDpy=0x55555558e510, attribs=<optimized out>) at ../egl-wayland/src/wayland-egldisplay.c:580
#4  0x00007ffff26acfa0 in  () at /usr/lib/libEGL_nvidia.so.0
#5  0x00007ffff264c71c in  () at /usr/lib/libEGL_nvidia.so.0
#6  0x00007ffff2ba4885 in GetPlatformDisplayCommon (platform=12760, native_display=0x55555558e510, attrib_list=0x0, funcName=0x7ffff2baad18 "eglGetDisplay") at ../libglvnd-v1.6.0/src/EGL/libegl.c:324
#7  0x00007ffff7a19357 in gdk_display_create_egl_display (native_display=0x55555558e510, platform=12757) at ../gtk/gdk/gdkdisplay.c:1484
#8  gdk_display_init_egl (self=0x5555555a1820, platform=12757, native_display=0x55555558e510, allow_any=0, error=0x5555555a17f8) at ../gtk/gdk/gdkdisplay.c:1667
#9  0x00007ffff79edb53 in gdk_x11_display_init_gl_backend (error=0x5555555a17f8, out_depth=0x5555555a18c4, out_visual=0x5555555a18c8, self=0x5555555a1820) at ../gtk/gdk/x11/gdkdisplay-x11.c:2975
#10 gdk_x11_display_init_gl (display=0x5555555a1820, error=0x5555555a17f8) at ../gtk/gdk/x11/gdkdisplay-x11.c:3013
#11 0x00007ffff7a198f0 in gdk_display_init_gl (self=0x5555555a1820) at ../gtk/gdk/gdkdisplay.c:1248
#12 gdk_display_prepare_gl (self=0x5555555a1820, error=0x0) at ../gtk/gdk/gdkdisplay.c:1320
#13 0x00007ffff79ec355 in gdk_x11_display_open (display_name=<optimized out>) at ../gtk/gdk/x11/gdkdisplay-x11.c:1479
#14 0x00007ffff7a15c62 in gdk_display_manager_open_display (manager=<optimized out>, name=0x0) at ../gtk/gdk/gdkdisplaymanager.c:431
#15 0x00007ffff777dda9 in gdk_display_open_default () at ../gtk/gdk/gdk.c:331
#16 gtk_init_check () at ../gtk/gtk/gtkmain.c:621
#17 gtk_init_check () at ../gtk/gtk/gtkmain.c:603
#18 0x00007ffff777dfee in gtk_init () at ../gtk/gtk/gtkmain.c:659
#19 0x0000555555557070 in main (argc=<optimized out>, argv=<optimized out>) at ../mutter/src/frames/main.c:56
```

然后我就在想看起来是 GTK4 的问题，我还去群里问了一下有没有 GNOME + NVIDIA 的用户，看看是我的问题还是 bug，不过没人理我，还差点把我恶心到了。然后我想了一下试了 `GDK_BACKEND=x11 nautilus` 发现也有一样的问题，就跑到 GTK 那边提了个 issue，结果那位有点出名的毒舌老哥跟我说看着不像是 GTK 的问题倒像是 nvidia 的问题，我也怀疑过，但我检查了一下和 nvidia 相关的都没什么变化，然后我去翻 glvnd 和 egl-wayland 的仓库也没翻出什么。换到 KDE 下面还是一样有问题。但我突然想到会不会和我设置的一些环境变量有关系，于是就去注销了一大片，结果就好了。最后我看了一下好像有 platform，发现是我设置过一个 `EGL_PLATFORM=wayland` 的环境变量，删掉这个就好了。

我想了一下这应该是我当初弄 Firefox 的硬件解码视频时候设置的，果不其然在 <https://github.com/elFarto/nvidia-vaapi-driver#firefox> 里面写了，看起来是因为这个变量导致 XWayland 程序加载 EGL 的时候把 platform 当成了 Wayland，不过我没想清楚为什么滚系统之前没有遇到这个问题。

总之这是个不能算 bug 的问题了，如果我在群里问的时候有人回我，我就能直接排除法发现是我自己配置的问题，结果提了 issue 以后发现不是上游的问题感觉很尴尬。想了一下还是决定把这个记在这里，因为我推测有很多人看了 `nvidia-vaapi-driver` 的文档，说不定也设置了这个变量然后遇到了同样的问题，记录下来方便搜到。
