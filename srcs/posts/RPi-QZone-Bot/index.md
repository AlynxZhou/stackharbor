---
title: 会发说说报时的树莓派
layout: post
categories:
    - 计算机
    - 树莓派
    - Python
tags:
    - 树莓派
    - Python
    - QQLib
comment: true
createdDate: 2016-08-14 15:40:04
---
<div class="alert-red">此方案已经被作者废弃且由于时间久远，记录的事物可能已经改变，作者不能保证此时页面内容完全正确，请不要完全参考。</div>

**由于WebQQ的验证码机制更新，QQLib的作者已于2016年8月17日更新源代码，本页也随之更新！请之前使用过的朋友用`# pip3 uninstall qqlib`卸载旧版，再重新安装最新版的QQLib，使用本页最新的Python脚本！**

-----------------------------------------------------------------

Hello，最近好久没有更新博客了，因为一直没有发现什么好玩的东西来写（其实就是因为懒）。那么前天终于发现了一个好东西，就是这个[QQLib for Python](https://github.com/gera2ld/qqlib/)，可以通过Python在QQ空间发说说的库。

<!--more-->

说到找这个东西的原因嘛……由于一些你懂得的原因，[Telegram](https://telegram.org/)被封锁是日益严重啊。我以前写了个非常有用的[TeleBot](https://github.com/Alynx/telebot/)跑在树莓派上，无奈身边用Telegram的人太少（~~无法装X~~），所以最开始想在树莓派上跑一个微信公众号来玩。然后发现微信的Api要求你指定一个域名，可是我树莓派跑在家里电信是不会白给我一个固定ip的（555……），为了解决这个问题我还特地造了个轮子，用Python自己封装了一个CloudXNS的Api用来定期ddns更新A记录，脚本写好了之后上网搜了一下发现有人说你在自己家里跑httpd被抓到是会被电信查水表的……虽然没有法律明文规定，但是就是这样，嗯，~~这背后一定有肮脏的PY交易~~（和谐）。于是微信公众号的计划就这样流产了……正好[L Giki](http://lgiki.xyz/)说他和他的朋友实现了一个QQ的聊天机器人，然而他们是PHP党……而且实现方法很复杂，主要是腾讯丧心病狂的加密……还有webQQ协议已经被腾讯阉割的不成样子了。百般无奈之时发现了这个GitHub项目，本着有轮子就不要自己造的原则，我决定写一个定期发说说的程序运行在树莓派上，那就是这样。

老规矩，这个脚本不是很长，下面贴上来吧，它的名字叫做`qclock.py`（2016-08-17更新）：

```Python
#!/usr/bin/env python3
#-*- coding: utf-8 -*-

### Filename: qclock.py
## Created by Alynx Zhou
# alynxzhou@gmail.com, http://alynx.xyz/

import os
import time
import qqlib
from qqlib import qzone

QQ_NUM = 1234567890
QQ_PASSWD = "MyPassWord."

qz = qzone.QZone(QQ_NUM, QQ_PASSWD)

try:
    qz.login()
except qqlib.NeedVerifyCode as nvc:
    # Get a verify code.
    verifier = nvc.verifier
    with open("verify.jpg", "wb") as verify_jpg:
        verify_jpg.write(verifier.image)
    print("Saved verify photo to verify.jpg, please open it by yourself.")
    vcode = input("Enter verify code here: ")
    try:
        # Test verify code.
        kw = verifier.verify(vcode)
    except qqlib.VerifyCodeError as vce:
        os.remove("verify.jpg")
        raise vce("Wrong verify code!")
    else:
        # Finish login.
        qz.login()
        os.remove("verify.jpg")

print("Login finished.")

unit = "°C"

def get_temp():
    # Get internal CPU temperature.
    with open("/sys/class/thermal/thermal_zone0/temp") as temp_open:
        temp = float(temp_open.read())/1000
    return temp

# Loop to get time and feed to qzone.
try:
    while True:
        tm = time.localtime()
        if tm.tm_min == 0:
            qz.feed("树莓派自动报时：\n现在是%d年%d月%d日 %d时整。\nCPU温度为%.2f%s。"%(tm.tm_year, tm.tm_mon, tm.tm_mday, tm.tm_hour, get_temp(), unit))
            print("Feeded at %d-%.2d-%.d %.2d:%.2d."%(tm.tm_year, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min))
        elif tm.tm_min == 30:
            qz.feed("树莓派自动报时：\n现在是%d年%d月%d日 %d时%d分。\nCPU温度为%.2f%s。"%(tm.tm_year, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, get_temp(), unit))
            print("Feeded at %d-%.2d-%.d %.2d:%.2d."%(tm.tm_year, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min))
        # Use 60 to keep only one feed in the minute.
        time.sleep(60)
except KeyboardInterrupt:
    exit()
```

当然，下载链接在[这里](/posts/2016/08/14/RPi-QZone-Bot/qclock.py)（2016-08-17更新）。

使用的时候先把`QQ_NUM`的值修改为你打算用来~~装X~~发说说的QQ号码，`QQ_PASSWD`的值则是你QQ的密码咯。这里建议各位不要使用自己用的QQ号做实验，而是另申请QQ号来作报时～不然谁知道腾讯哪天会不会找个什么理由关闭webQQ再冻结你的QQ号呢～

然后就是一句话的……不对！你还需要先安装QQLib啊，就像下面这样：

```bash
# pip3 install git+https://github.com/gera2ld/qqlib.git
```

然后一句话运行起来：

```bash
$ python3 ./qclock.py
```

保持它不要关闭，最好的办法还是[GNU Screen](https://www.gnu.org/software/screen/)啊～

每到整点和半点你的树莓派就会自动发一条说说报时并附上当前的CPU温度～如果你想做更多的话完全可以自己扩展我这个脚本。记住那个`time.sleep(60)`的60不是随便能改的，一分钟一次的循环不会特别耗资源，如果你改小了间隔，可能会导致一分钟内它走了两次循环，发了两条说说。时间的精确度保持在分钟级别。

然后，我用来测试的QQ号是`3530795351`，这是它的[QQ空间](http://user.qzone.qq.com/3530795351)，欢迎围观效果。

好啦，这个脚本就先介绍到这里了，我要去继续完善更多功能咯，首先还是要搞好我的Emacs，然后，我不是针对腾讯，我是说BAT这三个货都是辣鸡！

*Alynx Zhou*

**A Coder & Dreamer**
