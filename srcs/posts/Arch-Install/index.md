---
title: Arch Linux安装手札
layout: post
createdDate: 2016-04-07 06:26:23
comment: true
categories:
  - 计算机
  - Linux
  - Arch Linux
tags:
  - Arch Linux
---
<div class="alert-red">由于时间久远，记录的事物可能已经改变，作者不能保证此时页面内容完全正确，请不要完全参考。</div>

一入Arch深似海，不挂全需人品高～家里添了台联想昭阳E49,奔腾2020M,2G内存。早就想写一篇安装手札来做LOFTER的开篇了～我没那么多脑洞……代码还是有的……

Arch的安装出了名的难，没安过三次不要说你会Arch，但安装完你会对Linux有更好理解，如同打通任督二脉～我这次是第四次成功了～感谢好朋友Maxfan的帮助～另外LOFTER真的很不错～   

<!--more-->

下面正文开始～   

到[Arch的下载页面](https://www.archlinux.org/download/)下载最新的镜像，Windows下用Win32diskmanager写入U盘，Linux下用   

``` bash
# dd if=~/archlinux-2014.07.03.iso of=/dev/sdb
```

写入U盘。   

开机按<F12>选择U盘引导， *Boot Arch Linux(x86_64)* ，root自动登录。可以手机看[Arch Wiki新手指南](https://wiki.archlinux.org/index.php/Beginners%27_Guide_(%E7%AE%80%E4%BD%93%E4%B8%AD%E6%96%87)。~/有一份英文的install.txt，可以在tty2打开看。   

首先   

``` bash
# iw dev
```

记下无线接口名。本例为 *wlp2s0*    

``` bash
# wifi-menu wlp2s0
```

选择无线网，两次ok，键入密码连接。   

``` bash
# ping -c 3 www.baidu.com
```

确定是否已联网。   

``` bash
# cfdisk
```

听说高手都用fdisk分区，我习惯cfdisk，反正不从头分，在末尾的空闲分区划分70G/dev/sda3作为/，3G/dev/sda4swap，都是主分区。由于Arch用pacstrap安装基本系统，所以单/分区也能保留/home重装～只要删掉其他目录即可。千万不要乱按Bootable！给我惹了好大麻烦……   

``` bash
# partprobe
```

让内核重新读取分区表。   

``` bash
# mkfs.brtfs /dev/sda3
# mkswap /dev/sda4
# swapon /dev/sda4
```

根目录用btrfs，由于btrfs不能用交换文件，所以分交换分区并启用。   

``` bash
# mount /dev/sda3 /mnt
```

挂载到/mnt。   

``` bash
# nano /etc/pacman.d/mirrorlist
```

安装前需要编辑 */etc/pacman.d/mirrorlist* , mirrorlist文件也会被pacstrap复制到新系统。到[镜像列表生成器](https://www.archlinux.org/mirrorlist/)生成中国源列表添加到前面。   

``` bash
# pacman -Syy
```

更新pacman数据。   

``` bash
# pacstrap -i /mnt base base-devel
```

安装基本系统到/mnt，sudo什么的都在base-devel里。   

``` bash
# genfstab -p -U /mnt >> /mnt/etc/fstab
# nano /mnt/etc/fstab
```

生成fstab并检查一下。   

``` bash
# arch-chroot /mnt /bin/bash
```

chroot到新系统的bash shell，初步认为arch-chroot的特点是可以运行systemd。   

``` bash
# nano /etc/locale.gen
```

编辑本地化标准，去掉en_US.UTF-8和zh_CN.UTF-8前的#。执行   

``` bash
# locale-gen
```

应用更改。   

``` bash
# echo LANG=en_US.UTF-8 > /etc/locale.conf
```

设置LANG变量，在此设置中文会使tty乱码。   

``` bash
# ln -s /usr/share/zoneinfo/Asia/Shanghai /etc/localtime
```

设置时区，这里设为中国上海(UTC+8:00)。   

``` bash
# hwclock --systohc --utc
```

设置硬件时间为UTC，稍后再把Windows改为UTC。   

``` bash
# echo ShaX-Laptop > /etc/hostname
```

设置主机名为ShaX-Laptop。   

``` bash
# nano /etc/hosts
```

127.0.0.1一行最后按<TAB>键入ShaX-Laptop。   

``` bash
# pacman -S iw wpa_supplicant dialog
```

为新系统安装联网所需组件，第一次虚拟机安装时忘了配置网络，结果一pacman -S就满屏错误……   

``` bash
# pacman -S ntfs-3g vim zsh
```

安装必需软件，ntfs-3g可以让Linux读取NTFS。   

``` bash
# passwd
```

设置root密码。   

``` bash
# pacman -S grub
```

安装GRUB。为了能用Windows引导GRUB，选择安装到PBR。   

``` bash
# chattr -i /boot/grub/i386-pc/core.img
# grub-install --target=i386-pc --recheck --debug --force /dev/sda3
# chattr +i /boot/grub/i386-pc/core.img
# grub-mkconfig -o /boot/grub/grub.cfg
```

为了防止由于硬盘文件移动导致找不到GRUB核心文件，给它添加i(不可动)标记。建立GRUB配置文件。   

``` bash
# mount /dev/sda1 /mnt
```

挂载C盘到/mnt。   

``` bash
# mkdir /mnt/NST
# dd if=/dev/sda3 of=/mnt/NST/Linux.mbr bs=446 count=1
```

将前446字节的loader写到C:\NST\Linux.mbr。   

``` bash
# umount /mnt
# exit
```

退出chroot。   

``` bash
# umount /mnt
# reboot
```

不要忘了拔出U盘，进入Windows。   

打开regedit，定位到HKEY\_LOCAL\_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\TimeZoneInformation\\，新建DWORD值RealTimeIsUniversal，键值设为1。以应用UTC。   

下载安装EasyBCD软件，添加启动项→GNU/Linux→选GRUB2，名称添Arch Linux，添加。然后去C:\\NST\\把生成的另一个文件名复制，删掉，把Linux.mbr重命名为那个，因为EasyBCD默认有bug不能正确识别，所以我们手动替换即可。   

重启按<F1>进入BIOS，硬件时钟倒设8小时为UTC。   

选择Arch Linux进入GRUB引导。   

先用root登录，wifi\-menu连接网络，创建用户shax并添加密码。   

``` bash
# useradd -m shax
# passwd shax
```

参数-m的作用是自动在/home下建立以用户名为名的主文件夹。   
然后   

``` bash
# visudo
```

修改可以提权的用户，光标定位到root ALL=\(ALL\) ALL一行，按i键进入编辑模式，将该行修改为root ALL=\(ALL\) NOPASSWD: ALL，以关闭密码保护，在这一行下面添加shax ALL=\(ALL\) NOPASSWD: ALL，给shax开放权限。按<ESC>键退出编辑模式，输入:wq回车以保存并退出。   

``` bash
# logout
```

以shax登录。执行   

``` bash
$ sudo pacman -S xf86-video-intel xorg-server xorg-server-utils xorg-utils xorg-xinit alsa-utils xf86-input-synaptics
```

安装显声卡触摸板驱动和X接口。   

``` bash
$ sudo pacman -S acpi
```

安装高级电源管理支持(?)。   

```
$ sudo pacman -S gnome gdm wqy-microhei
```

安装GNOME3桌面环境和它对应的登录管理器GDM以及好看的中文字体文泉驿微米黑。   

``` bash
$ cp /etc/skel/.xinitrc ~/
```

复制一份X样板文件到主目录，不要加sudo，否则所有者会变成root。   

``` bash
$ nano ~/.xinitrc
```

去掉exec gnome-session前的#。   

``` bash
$ sudo systemctl enable gdm
```

开机自动启动登录管理器。   

``` bash
$ startx
```

启动X会话。   

在Settings→Language&Location里全部修改为汉语&中国。包括右上角的Login。   

右上角点击用户名选logout重新登录。   

登录后打开终端，输入   

``` bash
$ sudo pacman -S ibus-libpinyin
```

安装ibus-libpinyin中文输入法，之后去设置里添加即可。   

``` bash
$ sudo nano /etc/pacman.conf
```

添加如下以安装yaourt。   

````
[archlinuxcn]
#The Chinese Arch Linux communities packages.
SigLevel = Optional TrustAll
Server = http://repo.archlinuxcn.org/$arch
````


``` bash
$ sudo pacman -Syu yaourt
```

可以自动安装AUR工具yaourt。   

再次感谢好友Maxfan。   

欢迎关注[Arch Linux官网](https://www.archlinux.org/)   

[中文官网](http://www.archlinuxcn.org/)   

[中文wiki](https://wiki.archlinux.org/index.php/Main_Page_(%E7%AE%80%E4%BD%93%E4%B8%AD%E6%96%87))   

2014-07-28 17:00   

*Alynx Zhou*   

**A Coder & Dreamer**   

*搬运于我的LOFTER*   
