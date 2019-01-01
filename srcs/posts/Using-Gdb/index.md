---
title: 使用GDB进行简单的C语言程序调试
layout: post
comment: true
createdTime: 2017-03-15 19:24:33
categories:
  - 计算机
  - 编程
  - C 语言
tags:
  - C语言
  - GDB
---
事实证明有时候无聊的东西反而变成了一篇博文，谁叫恶趣味的C语言作业非要求用word写实验报告，本猹宁可先写成Markdown再复制粘贴过去。

<!--more-->

首先按照惯例应该介绍下GDB，搭配GCC使用的调试器，也是众多使用GCC为编译器的IDE的默认调试器（大概？），虽然无聊的实验报告模板还很搞笑的写了一个部分叫XXX调试器和VC6.0的比较（看戏的内心：我看过两年学生出生时间比VC6还晚的时候这些课还怎么拿VC6讲）。

所以掌握GDB的使用还是很有必要的，毕竟掌握某个使用GDB的IDE终究只是掌握了一个图形壳子，原理估计还是要深入一层到直接用GDB（强行接入正经模式）。

这里只是按照实验报告的要求写一下断点调试和单步执行的用法（PS：Word实在是太难用了，字号缩进和标题序号就没有按照我想要的方式工作过。

------------------------

# 准备

## 调试模式编译程序

首先应该明确调试的实现方式，GDB能够暂时的停下程序操作程序其实是通过向编译过程中的二进制流中添加钩子（HOOKS）来实现的。但是对于一个正常的，打算释出给用户使用的程序而言不应该以调试模式编译，因为这些添加进去的钩子不但会增大文件体积，还会降低程序的运行效率。所以GCC默认编译程序的时候时不会添加这个钩子的，需要的时候以`-g`参数来启用调试模式。

本例中以文件[Lab2.c](/posts/2017/03/15/Using-Gdb/Lab2.c)为例，使用命令

````
$ gcc -std=c11 -g -o lab Lab2.c
````

将它编译到可执行文件lab。

## 用GDB打开程序。

打开程序的命令很简单，就是`$ gdb lab`，但是有可能遇到的问题是程序要处理命令行参数，在使用GDB的时候是不能直接在命令行后面附加程序参数的，需要使用参数可以在打开GDB之后输入`set args 参数1 参数2 ...`附加上参数。

用GDB打开程序之后是一个交互界面，显示了一些信息和`(gdb)`这个提示符，表示你现在是在和GDB进行交互而不是程序。这个时候程序是不会运行的。

````
GNU gdb (GDB) 7.12.1
Copyright (C) 2017 Free Software Foundation, Inc.
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.  Type "show copying"
and "show warranty" for details.
This GDB was configured as "x86_64-pc-linux-gnu".
Type "show configuration" for configuration details.
For bug reporting instructions, please see:
<http://www.gnu.org/software/gdb/bugs/>.
Find the GDB manual and other documentation resources online at:
<http://www.gnu.org/software/gdb/documentation/>.
For help, type "help".
Type "apropos word" to search for commands related to "word"...
Reading symbols from lab...done.
(gdb)
````

# 调试

## 设置断点

首先使用`break`指令就可以添加一个断点，断点顾名思义就是程序运行到这个位置会暂停下来交给GDB，这样你才能够在程序运行到一半的时候查看一些可疑的变量，断点是针对函数使用的，比如我想在所有的printf()进入的时候暂停，可以使用：

````
(gdb) break printf
````

显示的是这样的：

````
(gdb) break printf
Breakpoint 1 at 0x400540
````

当然接下来可以继续添加更多的断点。

然后使用`run`就可以让程序跑起来：

````
(gdb) run
Starting program: /home/alynx/Homework/实验材料（学生版）/实验2/lab
[实验2说明]本练习运行一个三重for循环, 循环结构如下:
int count = 0;
for(int i=0; j<LoopEnd1; i++)
{
	//第一重循环体
	for(int j=0; j<LoopEnd2; j++)
	{
		//第二重循环体
		for(int s=0; s<LoopEnd3; s++)
		{
			count++;//第三重循环体
		}
	}
}

Breakpoint 1, 0x00007ffff7a86e00 in printf () from /usr/lib/libc.so.6
````

这时候使用`print`指令可以打印任意的内容：

````
(gdb) print printf
$1 = {<text variable, no debug info>} 0x7ffff7a86e00 <printf>
````

使用`bt`指令可以显示出程序的栈（Backtrace）：

````
(gdb) bt
#0  0x00007ffff7a86e00 in printf () from /usr/lib/libc.so.6
#1  0x0000000000400745 in main (argc=1, argv=0x7fffffffe078) at Lab2.c:26
````

输入`c`就是继续运行程序的意思。但是`step`指令的功能更加有用，顾名思义这个就是单步运行，每次只运行一行代码，这是个step-in的模式，即如果该行有函数则进入函数内一行一行运行，而`next`则是step-over的模式，即如果该行有函数会运行函数返回结果而不是进入函数内部。

````
(gdb) step
Single stepping until exit from function printf,
which has no line number information.
main (argc=1, argv=0x7fffffffe078) at Lab2.c:27
27	    scanf("%d", &LoopEnd1);
````

如果不输入指令直接回车，就是重复上一条指令，程序运行结束之后可以使用`quit`指令退出GDB。

-------------

好了本猹要去补作业了。

*AlynxZhou*

**A Coder & Dreamer**
