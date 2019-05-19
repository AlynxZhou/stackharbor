---
title: 批量改名——Hrit.py
layout: post
comment: true
createdTime: 2016-05-03 06:20:40
categories:
    - 计算机
    - 编程
    - Python
tags:
    - Python
    - Code Life
---
### 起因

记得以前读过一篇文章，大意说是懒人推动了社会进步，觉得言之有理。你想，要不是因为原始人懒得天天找吃的，怎么会出现培养出的农作物？要不是人们懒得自己搬庄稼走路，怎么会驯化马？要不是人们懒得自己养马，怎么会出现汽车？

<!--more-->

好了还是不要瞎扯了，总而言之对于我这么一个懒\+强迫症晚期\+半吊子程序员（？），当然在写网站的时候得把网站组织的有条理，所以肯定不能把所有的文章里的图片都堆到images/目录里就得了，幸好hexo提供了所谓的资源文件夹的方式，在新建Markdown文件的时候会自动建立同名的资源文件夹，只需编辑站点配置文件`_config.yml`开启如下项即可。

````yaml
post_asset_folder: true    #开启资源文件夹
````

OK\!这时候我天真的觉得只要把图片按数字命名为1\.jpg、2\.png之类的丢到对应资源文件夹里即可！然而生活远没有这么美好……首先在资源文件夹里的图片如果想在文章和首页的摘要里都显示的话就不能简单的用Markdown引用图片的语法，必须要用hexo的`asset_img`标签。

````
![图片名称](图片名称)
````

我开始方了，难道要我自己一个个手动打成这个标签……

还没完，如果你的访客想下载一些图片，如果恰好你不同资源文件夹里都有一个1\.jpg，他下载这两个的时候就会都保存为1\.jpg……那不就乱套了么……

所以我想要是能一次性给某个文件夹下的文件改名为 *文件夹名\_数字.扩展名* 并且还能自动输出成hexo标签该多好啊……本着Linux用户没有轮子自己造的精神我决定写个程序解决这个问题（Windows用户应该习惯于找别的东西解决吧……没有就自认倒霉一个一个手动改？想想就麻烦……）

### 行动

于是乎高三党为了珍惜劳动节唯一一天假期（**喵的一天也叫假期！！！**）果断决定写代码，开始想想这个简单工作适合shell script但是我不会写shell脚本也不会正则表达式……（观众：你说你一个不用Win只用Linux的人不会shell脚本和正则？！我：我真不会……别打我……）那似乎我会的剩下就是C和Python了……你叫我这么一个C语言水平已经退化到HelloWorld的家伙拿C写个程序处理字符串和文件？好吧刚才的观众们你们麻烦往死里打……所以只剩下Python比较熟练了。最后也是这么做的。

### 成果

那这样就轮到今天的主角登场了——hrit！

为什么叫这个名字呢？因为它是Hexo Rename Images Tool！当然使用了os、time和argprase的它其实能给所有文件命名……（hrit：人家只是个不到100行代码的小脚本……我：嘘……观众：他在那！！！）

所以先贴代码：

````Python
#!/usr/bin/env python3
#-*- coding:utf-8 -*-

### Filename:hrit.py
## Created by Alynx Zhou
# alynxzhou@gmail.com, http://alynx.xyz/

import os
import time
import argparse

### Deal with the argument.
parser = argparse.ArgumentParser(description="hrit -- Hexo Rename Images Tool.")
parser.add_argument("-f","--forehead",action="store",type=str,help="The string will be added to the begin of the file name.Default is the directory name(dir).")
parser.add_argument("-m","--middle",action="store",default='_',type=str,help="This will be put between the head and the tail.Default '_'")
parser.add_argument("-n","--number",action="store_true",help="If this argument is set,the file name between the head and the last dot will be set as an increasing number.")
#parser.add_argument("dir",action="store",nargs='?',default="./",type=str,help="The directory to work.")
parser.add_argument("dir",action="store",type=str,help="The directory to work.")
args = parser.parse_args()

### Remember the original path.
opath = os.getcwd()

### Change directory.
print("Entering directory %s …"%(args.dir))
try:
    os.chdir(args.dir)
except FileNotFoundError:
    print("ERROR!Can't find directory %s !"%(args.dir))
    print("Quiting…")
    exit()
curdir = os.getcwd()

### Ask.
print("Now it is going to work in ")
print("=====================================================")
print("%s"%(curdir))
print("=====================================================")
print("Please check weither it is the exact directory you want!")
print("Work in other directories can be dangerous and hard to fix(For instance,/,/home,/usr,etc.)!")
answer = input("Continue?[y/N] ")
if answer != 'y' and answer != "yes":
    print("Quiting…")
    os.chdir(opath)
    exit()

print("Wating 5 seconds to regret…")
ilist = ["5…","5 4…","5 4 3…","5 4 3 2…","5 4 3 2 1…"]
for i in ilist:
    print("%s\r"%(i),end="")
    time.sleep(1)
print("5 4 3 2 1…Start!\n")

### Get dir.
if args.forehead == None:
    forehead = curdir.split(os.sep)[-1]
else:
    forehead = args.forehead

### List files.
filelist = os.listdir(curdir)
print("Found files:%s.\n"%(filelist))

### Rename.
if args.number == True:
    ## Number name.
    number = 1
    for file in filelist:
        filel = file.split('.')
        if len(filel) == 1:
            ## Without extand name.
            newname = forehead + args.middle + str(number)
        else:
            ## With extand name.
            newname = forehead + args.middle + str(number) + '.' + filel[-1]

        os.renames(file,newname)
        print("Renamed %s to %s."%(file,newname))
        number = number + 1
else:
    for file in filelist:
        newname = forehead + args.middle + file

        os.renames(file,newname)
        print("Renamed %s to %s."%(file,newname))

### Get new file list.
newlist = os.listdir(curdir)
print("\nNow you can copy the follow hexo tags to your markdown post.\n")
for new in newlist:
    print("{%% asset_img %s %s %%}\n"%(new,new))

### Go back to original path.
os.chdir(opath)
````

好了亲爱的观众们我知道你们又要抱怨你们的Vim/Emacs设置了代码自动缩进粘贴Python很麻烦……（你说你用Sublime Text或者Notepad++？好吧自便。你说你用记事本？这我就不能忍了，哪凉快哪呆着去！→\_→）我就大发慈悲把源文件传上来吧，右键点击[这里](/posts/2016/05/03/Hrit/hrit.py)另存为即可。然后拷贝到你的PATH中任意一个目录。

接下来说用法～什么用法？直接执行hrit -h不就有用法了吗？

总而言之hrit会以一个文件夹为单位把里面所有的文件按照指定的规则重命名，所以最后必须接一个文件夹参数比如\./test/，没有设置默认\./的原因是为了安全～hrit将文件名分为五部分操作，前缀、连接符、后缀、英文句点和扩展名。然后如果指定了 *-f 前缀* 的话文件都会被添加上同一前缀，没有-f则默认以当前目录名为前缀。 * -m 连接符* 指定连接符，默认是“\_”。最后hrit会把原本的文件名（包括句点和扩展名）一起接上作为后缀。如果开启了 *-n* 它会用从1开始递增的数字作为后缀，并连接上句点和原本的扩展名（如果你非要把文件命名为aaa\.bb\.c那hrit只会把\.c连接上……）。

下面示范一下，进入我饱经摧残的test/目录。

````
$ cd test/ && ls -alh
total 8.0K
drwxr-xr-x 2 shax shax 4.0K May  2 21:58 .
drwxr-xr-x 3 shax shax 4.0K May  2 11:44 ..
-rw-r--r-- 1 shax shax    0 May  2 21:58 aaa
-rw-r--r-- 1 shax shax    0 May  2 21:58 bb.b
-rw-r--r-- 1 shax shax    0 May  2 21:58 c.c.c
````

下面执行下

````
$ hrit ./
Entering directory ./ …
Now it is going to work in
=====================================================
/home/shax/ProgramPractice/hrit/test
=====================================================
Please check weither it is the exact directory you want!
Work in other directories can be dangerous and hard to fix(For instance,/,/home,/usr,etc.)!
Continue?[y/N] y
Wating 5 seconds to regret…
5 4 3 2 1…Start!

Found files:['aaa', 'c.c.c', 'bb.b'].

Renamed aaa to test_aaa.
Renamed c.c.c to test_c.c.c.
Renamed bb.b to test_bb.b.

Now you can copy the follow hexo tags to your markdown post.

![test_c.c.c](test_c.c.c)

![test_bb.b](test_bb.b)

![test_aaa](test_aaa)
````

为了防止搞错了目录设置了一个确认和5秒反悔时间。

下面试试 `-f`

````
$ hrit -f xxx ./
Entering directory ./ …
Now it is going to work in
=====================================================
/home/shax/ProgramPractice/hrit/test
=====================================================
Please check weither it is the exact directory you want!
Work in other directories can be dangerous and hard to fix(For instance,/,/home,/usr,etc.)!
Continue?[y/N] y
Wating 5 seconds to regret…
5 4 3 2 1…Start!

Found files:['test_c.c.c', 'test_bb.b', 'test_aaa'].

Renamed test_c.c.c to xxx_test_c.c.c.
Renamed test_bb.b to xxx_test_bb.b.
Renamed test_aaa to xxx_test_aaa.

Now you can copy the follow hexo tags to your markdown post.

![xxx_test_c.c.c](xxx_test_c.c.c)

![xxx_test_aaa](xxx_test_aaa)

![xxx_test_bb.b](xxx_test_bb.b)
````

最后 `-n`

````
$ hrit -n ./
Entering directory ./ …
Now it is going to work in
=====================================================
/home/shax/ProgramPractice/hrit/test
=====================================================
Please check weither it is the exact directory you want!
Work in other directories can be dangerous and hard to fix(For instance,/,/home,/usr,etc.)!
Continue?[y/N] y
Wating 5 seconds to regret…
5 4 3 2 1…Start!

Found files:['xxx_test_c.c.c', 'xxx_test_aaa', 'xxx_test_bb.b'].

Renamed xxx_test_c.c.c to test_1.c.
Renamed xxx_test_aaa to test_2.
Renamed xxx_test_bb.b to test_3.b.

Now you can copy the follow hexo tags to your markdown post.

![test_2](test_2)

![test_1.c](test_1.c)

![test_3.b](test_3.b)
````

不错吧，这样以后直接把所有图片拷贝到资源文件夹里用`hrit -n ./xxxx/`即可批量命名了～然后直接把输出的标签拷贝到Markdown文件～

最后说一个小插曲，开始我设置了询问继续和五秒倒计时的时候其实并不觉得有什么用……直到某次调试时不小心在home下执行了hrit……幸好有5秒按Ctrl\+C，顿时觉得自己真是太机智了。

*Alynx Zhou*

**A Coder & Dreamer**
