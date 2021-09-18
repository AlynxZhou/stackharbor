---
title: 通过 USB HID over AOAv2 在 scrcpy 里模拟真实键盘
layout: post
#comment: true
created: 2021-09-18T21:08:08
categories:
  - 计算机
  - 编程
  - Android
tags:
  - Android
---
三年前（2018 年）我在 scrcpy 的 GitHub 仓库里提了 [这个 issue](https://github.com/Genymobile/scrcpy/issues/279)，因为我当时发现这个项目能把手机投屏到电脑上，也就是说我就可以在 Linux 下面通过 Android 手机聊 QQ 了（我当时大概也许还有高强度聊 QQ 的需求），不过试了之后发现很难用，因为它和直接在手机上插键盘不一样，显示的还是软键盘，虽然能通过电脑键盘触发输入法，但是却不能用数字键选词。我当时也不太懂，于是就发 issue 问开发者，[@rom1v][@rom1v] 回复说有个叫 HID over AOA 的办法可以实现，但是对有些设备来说有 bug，同时也需要有人花时间读 USB 规范然后做把 SDL event 转换成 HID event 的工作。我又想那能不能直接用电脑的输入法生成字符然后传给手机，[@rom1v][@rom1v] 表示现在就是这么做的，但是 Android 相关的 API 限制只能发送 ASCII 的字符，所以也行不通。

我当时稍微了解了一下这些相关的东西，不过显然超出了我的能力范围，于是这件事我就搁置了。一直到最近或者准确的说就是上周和 Hackghost 跟我提起说苹果似乎打算做手机投屏到电脑的功能，然后又说华为好像有个现成的。不过我对这些一向是漠不关心的，这些厂商就是又懒又坏的典型，不会做一个 Linux 版的客户端的。如果他们自己做不了或者不打算做，那就应该公开一点，让能做的人来做而不是藏着掖着。然后就是我们讨论了一气关于成本到底谁付了谁亏了的问题，我坚持认为厂商赚得已经够多了，成本和利润 Linux 用户在买手机的时候也是照样付的，只是这些人贪得无厌能少付出成本就少付出一点而已。最后我说已经有能做的人做了 scrcpy 这个项目出来，投屏完全没问题，支持各种平台，美中不足就是输入体验不太好。这时候我又想已经过去很久了，不如我再试试去看看能不能解决输入体验的问题，于是就回去翻了这个 issue。

运气不错，翻过去看到在 2019 年年初的时候 [@amosbird][@amosbird] 已经写了一份代码，他自称是能用的，然而不知道为什么当初没有合并进主线，现在多半也是跑不起来。我一开始想我把他这个在当前的 HEAD 上重构一下就好了，于是开始读他的代码。一开始还是没什么头绪，看起来他似乎写了不止一个功能，但是都在一个 commit 里面，又没什么关于思路的注释。随后我加上了他的 Telegram，不过他本人表示时间有点久了他也不记得自己写的代码都是什么意思（笑）。于是我只能硬着头皮啃了，好在我现在的经验比以前涨了很多，然后再同时啃 USB 和 Android 的协议，配合一些搜到的其他资料，最终了解了大概是怎么回事。最开始本来以为简单地合并一下代码就可以了，没想到还发现了他代码里的错误，基本是变成重写了。花了头两天事件让程序跑起来，然后用了几天调整成和现有代码一致的风格以便合并进去。 *总之很是有一点新手村出来遇到 BOSS 暂时撤退，升级打怪三年之后杀回来的感觉。*

既然是 USB HID over AOAv2，那很显然需要知道 USB HID 是怎么回事，USB 官方有一个 [很长的 PDF][Device Class Definition for Human Interface Devices] 规定怎么成为一个合法的 HID 设备，说实话，看不太下去。如果你想要查一些有帮助的例子，直接查 AOAv2 多半是没戏的，只有 [Android 自己一个惜字如金的文档页](https://source.android.com/devices/accessories/aoa2)，我的经验就是你找那些主题是用单片机模拟键盘鼠标的文章，他们的目标和这个基本是一致的。不过我说好不碰硬件的话看来是算作废了。

基本上成为一个 USB HID 设备需要你发送一大堆的描述符到主机，不过我们这里有点不一样，因为 Android 设备连接电脑的时候，Android 是 USB 从设备，电脑是主设备，而 AOAv2 是从主机反向发数据到从设备，它不要求我们发送一大堆 USB 的描述符，只要向 Android 注册一个设备，发送 HID 的报告描述符，再发送 HID event，再注销就好了。这部分可以通过 libusb 这个库来实现 USB 的数据包传输，然后把 Android 的几个命令封装成函数就可以了，基本是在 <https://github.com/AlynxZhou/scrcpy/blob/dev/app/src/aoa_hid.c#L155-L246> 这部分。

基础的 API 有了之后则是具体的发什么数据包了，HID 的数据实际上就是由 byte 组成的 buffer，首先就是报告描述符，这个描述符是让主设备知道每个发过来的 event 里面的每个 byte 都是什么含义，键盘的描述符其实相对是比较固定的，在 [Device Class Definition for Human Interface Devices][Device Class Definition for Human Interface Devices] 这个 PDF 里面其实给了一个最简单的 USB 键盘的例子，这个也是保证在 BIOS 里面能正常使用 USB 键盘的最小集合，是在 Appendix B: Boot Interface Descriptors 下面的 B.1 Protocol 1 (Keyboard) 和 Appendix E: Example USB Descriptors for HID Class Devices 下面的 E.6 Report Descriptor (Keyboard)。不过有时候光知道这些还不够，比如报告描述符里面大部分都是两个 byte 一句话，第一个 byte 表示的是类别而第二个表示的是具体的值，后面的大概很好理解，但是第一个 byte 是怎么算出来的可能需要了解，这需要看那个 PDF 里 8. Report Protocol 这一节了，或者中文的话可以看 [这篇知乎文章](https://zhuanlan.zhihu.com/p/41960639)，然后你就会明白为什么有时候看起来数字不一样结果含义却一样了，因为其实第一个 byte 的每个 bit 都是有分别的含义的。然后就是对于 Usage Tag 这个有很多，被放在 [另一个单独的 PDF](https://usb.org/sites/default/files/hut1_22.pdf) 里面了。

基本上我还是把 <https://github.com/AlynxZhou/scrcpy/blob/dev/app/src/hid_keyboard.c#L28-L144> 这段代码贴过来好了，详细的说明我加在了注释里面：

```c
unsigned char kb_report_desc_buffer[]  = {
    // Usage Page (Generic Desktop)
    // 键盘这个字段应该是 Generic Desktop，为啥是这个我也不知道。
    0x05, 0x01,
    // Usage (Keyboard)
    // 也不用我解释了吧，自己查表去。
    0x09, 0x06,

    // Collection (Application)
    // 然后基本上 USB HID 描述符要有不同的 Collection，
    // 代表你发到主机的一组数据，
    // 一个设备最少有一个 Collection 吧，不然也没意义了。
    // 为啥是 Application 好像是因为 Keyboard 的 Usage Page tag 在这个里面。
    0xA1, 0x01,
    // Report ID (1)
    // 你会发现最基础的那个键盘示例里面没有这个字段，
    // 实际上当你只发一种数据的时候这个字段可以省略。
    // 但是现在是个机械键盘都带媒体控制按键吧，那个要算另一个 Collection 的，
    // 所以就通过 Report ID 区分，同时你发数据的时候第一个 byte 就得说明自己发的是哪个
    // Report ID。所以不要问为什么网上说 USB HID 键盘一个事件 8 个 byte 我们却发了 9 个。
    0x85, 0x01,

    // Usage Page (Keyboard)
    // 这里注意 Usage Page 和 Usage 不一样咯。好像这个文档里也叫 Key Codes 来着。
    0x05, 0x07,
    // Usage Minimum (224)
    // 一般来说一组报告数据包含很多用途，你不可能把所有的用途都列出来，给个用途范围就可以了。
    // 这里表示最小的用途 tag 是 0xE0，应该是 Left Control 的意思。
    0x19, 0xE0,
    // Usage Maximum (231)
    // 最大的用途 tag，是 Right GUI 键。这个 byte 的含义就是键盘上的八个修饰键。
    // Left Control，Right Control，Left Alt，Right Alt，Left Shift，Right Shift，
    // Left GUI，Right GUI。
    // 不过一定要记住 16 进制数转成 2 进制时候最右边是第 0 位，也就是说上边的顺序要倒过来。
    0x29, 0xE7,
    // Logical Minimum (0)
    // 每个 bit 的逻辑最小值当然是 0 啦。
    0x15, 0x00,
    // Logical Maximum (1)
    // 按下去就变成最大值 1。
    0x25, 0x01,
    // Report Size (1)
    // 每个数据只占 1 bit。
    0x75, 0x01,
    // Report Count (8)
    // 一共 8 个数据。
    0x95, 0x08,
    // Input (Data, Variable, Absolute): Modifier byte
    // 关于 Input tag 是什么意思太长了，自己查去吧，总之和上面那些要符合。
    0x81, 0x02,

    // 下面这段是厂商保留位，一般只要写一个为 0 的 byte 就可以了。
    // Report Size (8)
    0x75, 0x08,
    // Report Count (1)
    0x95, 0x01,
    // Input (Constant): Reserved byte
    0x81, 0x01,

    // 这部分表示主设备返回的关于 LED 灯亮的信号，
    // HID 键盘自己是不保存什么大写锁定的状态的。
    // 不过我们毕竟不是真的键盘，这部分抄一下就完事了，程序直接忽略。
    // 总之键盘上有 5 个灯，和上面的修饰键差不多，一个 bit 代表一个灯。
    // Usage Page (LEDs)
    0x05, 0x08,
    // Usage Minimum (1)
    0x19, 0x01,
    // Usage Maximum (5)
    0x29, 0x05,
    // Report Size (1)
    0x75, 0x01,
    // Report Count (5)
    0x95, 0x05,
    // Output (Data, Variable, Absolute): LED report
    0x91, 0x02,

    // 5 个 bit 对不齐，这 3 个 bit 是为了凑整的。
    // Report Size (3)
    0x75, 0x03,
    // Report Count (1)
    0x95, 0x01,
    // Output (Constant): LED report padding
    0x91, 0x01,

    // 下面就有意思了，键盘上有 101 个普通键，你肯定是不会同时按下 101 个键的，
    // 所以也不需要像修饰键那样每个 bit 代表一个键
    //（当然理论上你写个描述符说我这个键盘就是这样的也未尝不可啦），
    // 所以实际上这里返回的是一个数组，每个 byte 代表一个被按下去的键的键码。
    // Usage Page (Key Codes)
    0x05, 0x07,
    // Usage Minimum (0)
    // 用途最小值从不代表任何键的空值 0 开始。
    0x19, 0x00,
    // Usage Maximum (101)
    // 这里就是 101 啦，标准键盘上的按键数量，如果要支持非标准键盘自己查表去。
    0x29, HID_KEYBOARD_KEYS - 1,
    // Logical Minimum (0)
    // 因为每个 byte 放的都是键码，键码最小值是 0。
    0x15, 0x00,
    // Logical Maximum(101)
    // 最大值就是第 101 个键。
    0x25, HID_KEYBOARD_KEYS - 1,
    // Report Size (8)
    // 每个值都是 1 byte。
    0x75, 0x08,
    // Report Count (6)
    // 最基础的 USB 键盘一次最多能返回 6 个被同时按下的按键，
    // 你的 USB 键盘实际上很可能会告诉电脑它能报告更多，
    // 不过我们毕竟不是真的键盘，6 个键应该能满足大部分情况了。
    0x95, HID_KEYBOARD_MAX_KEYS,
    // Input (Data, Array): Keys
    // 这里也查表去吧，总之变成了 Array 而不是 Variable。
    // 一个 Collection 应该是只能有一个 Array，必须在末尾来着。
    0x81, 0x00,

    // End Collection
    // 这样我们代表正常键盘的 Collection 就结束了。
    0xC0,

    // Usage Page (Consumer)
    // 下面则是代表媒体控制的按键，他们属于另一个用途页了。
    0x05, 0x0C,
    // Usage (Consumer Control)
    // 这些键在另一个用途下面。
    0x09, 0x01,

    // Collection (Application)
    // 一个新的 Collection，数据和之前不一样了。
    0xA1, 0x01,
    // Report ID (2)
    // 一个新的 Report ID，
    // 所以这种 event 是 1 byte Report ID 和 1 byte 键码一共 2 byte。
    0x85, 0x02,

    // Usage Page (Consumer)
    // 下面的用途都算这里。
    0x05, 0x0C,
    // 这些和修饰键基本是一个意思，每个 bit 代表一个按键，不过这些用途不连贯了。
    // Usage (Scan Next Track)
    0x09, 0xB5,
    // Usage (Scan Previous Track)
    0x09, 0xB6,
    // Usage (Stop)
    0x09, 0xB7,
    // Usage (Eject)
    0x09, 0xB8,
    // Usage (Play/Pause)
    0x09, 0xCD,
    // Usage (Mute)
    0x09, 0xE2,
    // Usage (Volume Increment)
    0x09, 0xE9,
    // Usage (Volume Decrement)
    0x09, 0xEA,
    // 和修饰键差不多啦下面。
    // Logical Minimum (0)
    0x15, 0x00,
    // Logical Maximum (1)
    0x25, 0x01,
    // Report Size (1)
    0x75, 0x01,
    // Report Count (8)
    0x95, 0x08,
    // Input (Data, Array)
    0x81, 0x02,

    // End Collection
    0xC0
};
```

所以基本上我们构建的虚拟键盘就告诉主设备它是这么报告数据的。里面除了要注意具体的数据之外第一个 byte 要放 Report ID 之外也没什么。然后就是怎么把 SDL 的事件转换成 HID 事件了，一开始看 [@amosbird][@amosbird] 的代码，发现他把 HID event 理解成了和 SDL event 一样的东西了——有一个类似修饰键的数据和一个按下抬起的数据和一个具体哪个键的数据，用户按一个键就生成一个针对这个键单独的 event——但是其实不是，HID 键盘并不会直接告诉你哪个键按下还是抬起了，从上面的报告描述符也能看出来，实际上它是一个基于顺序的协议，比方说我按下了 C，给主机的事件可能是 `C键 00 00 00 00 00`，又按下了 B，再发一个 `C键 B键 00 00 00 00`，抬起来 C，再发一个 `B键 00 00 00 00 00`，然后主机对比之前和之后的事件，得到“C 抬起了”或者“B 按下了”的信息，这才是我们司空见惯的“键盘事件”。所以在程序里面我们需要做个反向操作，把单独针对某个按键的按下抬起的数据变成一个“有哪些键按下”的数据。这里其实很简单，我们内部用一个数组保存当前的按键状态，每次有 SDL 的按键事件发来就更新状态（**不是所有按键事件都是那 101 个键哦**），然后遍历这个数组，就可以利用哪些索引对应的值是 `true` 生成 HID 事件的内容了，同时 HID 也不要求发送的按键在数组里的顺序和按下的顺序一致，而且 SDL 的 scancode 和 HID 的值是一致的。对于修饰键更简单，SDL 每个事件里面的修饰键和 HID 一样，都是包含当前所有修饰键的状态，只是具体的哪个 bit 表示哪个键不一样，转换一下就可以了（<https://github.com/AlynxZhou/scrcpy/blob/dev/app/src/hid_keyboard.c#L195-L223>）。**但是千万不要因为发来的按键不是那 101 个键就直接忽略掉整个事件，因为可能用户只是单独按了一下修饰键，所以如果要跳过只要跳过更新按键状态就好了**（<https://github.com/AlynxZhou/scrcpy/blob/dev/app/src/hid_keyboard.c#L225-L269>）。

可能有聪明的小朋友要问了，你这最多只能发六个键，我按下七个怎么办？自己好好读一下 HID 协议就行了，这种情况下需要回一个 phantom state，具体就是修饰键一切照常，六个 key 全返回 `0x01`（<https://github.com/AlynxZhou/scrcpy/blob/dev/app/src/hid_keyboard.c#L250-L259>）。

基本上看到这里已经可以成功的给 Android 手机发 HID 键盘事件了，另外就是如果你用桌面环境的话，媒体按键应该会被桌面环境拦截，所以其实我没有发媒体事件，scrcpy 是用组合键单独处理了一部分功能。然后就是一些收尾，比如 [@amosbird][@amosbird] 忘记在程序结束之前取消注销 HID 设备了，这会导致如果你不拔掉 USB 线，触摸用的软键盘就一直出不来。

最后 [@rom1v][@rom1v] 表示 scrcpy 是在单独的线程里处理输入的，所以我也给 AOA 单独起了一个线程，这个只要照着 scrcpy 原本 controller 的线程抄一个就可以了。

你要是问为什么不把鼠标也用 HID 的方式模拟进去的话，我可以告诉你我也试过了，效果并不算好，AOA 是可以注册不止一个 HID 设备的，只要你分配不同的 ID 并作为参数发过去就可以了，这一部分代码我都留好了。但是一个主要的问题是 HID 鼠标只汇报 X 和 Y 的变化量，导致比方说我把鼠标从 SDL 的窗口挪走，MotionEvent 停止，HID 鼠标就会停在边框上，这时候我再从另一个方向挪进窗口，HID 鼠标的指针和你本机的指针就不再同步了，体验不如 scrcpy 自己注入事件的方式，所以我放弃了。

Windows 下面 libusb 似乎不能很好的连接 Android 手机发送数据，这个我没什么办法，看起来是个 [陈年老 bug](https://libusb-devel.narkive.com/vENuKzdR/getting-the-serial-number-of-the-camera-fails)，考虑到我一开始的目的只是我自己能在 Linux 下面方便地聊 QQ，做成这样我觉得就可以了，我的代码逻辑正确，即使要修复，也不至于动 scrcpy 这部分而是动 libusb，[@rom1v][@rom1v] 也表示现在就很不错了。

当然因为依赖 USB，所以你利用 ADB over WiFi 的话就没办法用这个功能了，不过我用电脑时候一般会给手机充电，也无所谓。

提交的 PR 链接是 <https://github.com/Genymobile/scrcpy/pull/2632/files>，感觉是个大工程，最后实际上也就修改了一千行？不过确实挺难的。

*Alynx Zhou*

**A Coder & Dreamer**

[@rom1v]: https://github.com/rom1v
[@amosbird]: https://github.com/amosbird
[Device Class Definition for Human Interface Devices]: https://www.usb.org/document-library/device-class-definition-hid-111
