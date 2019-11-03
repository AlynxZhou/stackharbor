---
title: Linux下使Steam调用Bumblebee使用独显
layout: post
comment: true
createdDate: 2017-02-16 12:50:22
categories:
  - 计算机
  - Linux
  - Steam
tags:
  - Linux
  - 双显卡
  - Bumblebee
  - Steam
---
买了个罗技G502，想想这鼠标狙击键要在fps游戏里才能发挥作用于是入了l4d2的坑，Steam购买，没想到P530核显实在是吃不动这货，FPS上30都很困难，于是研究了一下怎么样调用Bumblebee来使用独显。

<!--more-->

要使用独显首先你得保证你有个独显（这不是废话吗），我的独显是Nvidia Quadro M1000M，虽然是工作站显卡，但是玩游戏应该能达到GTX960左右的性能吧，然后你得安装Bumblebee（我只弄过N卡的，A卡不清楚），但是不是简单的安装Bumblebee这个包，按照[ArchWiki](https://wiki.archlinux.org/index.php/Bumblebee_(%E7%AE%80%E4%BD%93%E4%B8%AD%E6%96%87))，需要安装的包有`bumblebee bbswitch primus virtualgl lib32-primus lib32-virtualgl mesa nvidia nvidia-settings lib32-nvidia-utils lib32-mesa`，各个包的作用在下面：

- bumblebee

主要的程序啦，用来建立一个独立的X服务器做渲染。

- bbswitch

用来切换nvidia卡的开关状态，有了它才能保证不用的时候关闭nvidia卡省电。

- primus virtualgl

这两个分别是不同的桥接器，可以二选一，也可以都用，ArchWiki介绍的是virtualgl的optirun，但是primus是未来的默认选项，性能更好，Steam也推荐用primus。

- lib32-primus lib32-virtualgl

这两个是32位库，因为Steam和大部分上面的游戏都是32位的。

- mesa nvidia nvidia-settings lib32-nvidia-utils lib32-mesa

mesa是开源的OpenGL标准实现，另外的则是 ~~I卡的驱动（官方开源）和~~ N卡的闭源驱动（性能较好），N卡的设置，以及它们对应的32位库。

**更新**：I 卡推荐用内核实现了，不用装那个了。

接下来将你的用户加入bumblebee组，启动bumblebeed：

````
# gpasswd -a user bumblebee
# systemctl enable bumblebeed --now
````

然后测试：

- 64位`$ optirun glxspheres64`

- 32位`$ optirun glxspheres32`

需要独显运行的程序直接用`$ optirun %command%`就好啦，当然把optirun换成primusrun就是用primus啦。

对于Steam单独的游戏设置，只要在游戏上右键点*Properties*，打开的窗口选择`SET LAUNCH OPTIONS`，输入`primusrun %command%`，运行游戏的时候就是用primusrun运行了。我这里l4d2可以稳定60FPS。

但是还有一些需要启动器启动的，比如Minecraft，采用Hello Minecraft Launcher，用`$ optirun java -jar HMCL.jar`启动了启动器，但是启动器却不会用optirun去运行游戏本体，解决办法也很简单：

首先`$ optirun bash`，在这个bash里面用`$ optirun java -jar HMCL.jar`启动启动器，再打开游戏就是独显啦！

~~本来`$ optirun status`是应该能查看N卡状态的，但是Arch似乎没法运行这个命令，可以用`$ cat /proc/acpi/bbswitch`查看ON/OFF。~~这里是我记错了，应该是用`$ optirun --status`查看N卡状态。

对于一些版本比较老的bumblebee，optirun开启了独显后没法自动关掉它，是一个bug，手动关闭需要：

````
# rmmod nvidia_modeset nvidia && sudo tee /proc/acpi/bbswitch <<<OFF
````

但是新版已经解决了这个bug。

如果你想开启N卡的设置，不能简单地运行nvidia-settings，因为这时候N卡并没有开启，也没有连接到当前的桌面上，使用`$ optirun nvidia-settings -c :8`就可以开启了。

参考了[Steam的说明](https://support.steampowered.com/kb_article.php?ref=6316-GJKC-7437)，现在剩余的问题就是玩游戏太热了。

*Alynx Zhou*

**A Coder & Dreamer**
