---
title: 使用 Moment 处理 JavaScript 与 YAML 的时区转换
layout: post
comment: true
createdTime: 2019-01-01 21:00:00
zone: Asia/Tokyo
categories:
  - 计算机
  - 编程
  - JavaScript
tags:
  - 计算机
  - 编程
  - JavaScript
---
开始之前先来一段惊喜：我是在中国标准时间 21 点写的这篇文章，但我在 front matter 写的是 21:00:00 并解析为东京时间。

<!--more-->

今天想给 Hikaru 添加文章时间显示时区的功能，本来以为只要无脑调用 Moment.js 就好了，但发现其中坑有很多，本来我就很不擅长计算时区转换，难度变得很大，好在后来处理完了，记录一下遇到的坑。

首先 Moment 是不支持地区解析和时区显示的，需要安装 `moment-timezone` 这个扩展包才可以处理，文档在[这里](https://momentjs.com/timezone/docs/)。

然后就是 js-yaml 开发者认为是标准但我觉得是 Bug 的[问题](https://github.com/nodeca/js-yaml/issues/91)。我在 front matter 里写的时间是不包含时区的，比如 `2018-12-31 16:00:00`，按照正常的解析方式（就是直接 `new Date("2018-12-31 16:00:00")`），应当把这个时间当作 localtime 来看待，也就是 UTC+8h 的时间，但偏偏 js-yaml 的开发者认为按照 YAML 标准应当把这个时间当作 UTC 来处理（问题是他 **根本解析不了附加时区的表示方式啊！**）然后时间就错乱了，不过还好，JS Date() 对象内置的 `getTimezoneOffset()` 方法可以获取到 UTC 与 localtime 的分钟差（和标注的 UTC+8h 正好相反，这个是 -8h，在这里正好把这个错误的 UTC 换算回正确的 UTC）。具体的代码如下。

```javascript
const yaml = require("js-yaml")
const d = yaml.safeLoad("2018-12-31 16:00:00")  // -> 2018-12-31T16:00:00.000Z
new Date("2018-12-31 16:00:00") // -> 2018-12-31T08:00:00.000Z
d.getTimezoneOffset() // -> -480
new Date(d.getTime() + d.getTimezoneOffset() * 60 * 1000) // -> 2018-12-31T08:00:00.000Z
```

接下来遇到另一个问题，Nunjucks 似乎对传递参数有很大限制，比如 `moment` 既是函数也有成员，我没办法在 Nunjucks 里面调用它的 `moment.tz.guess()` 成员，所以只能把设置地区单独做成文章参数传进去。

然后发现这样有一个问题，比如我在日本写文章的时间是 21 点，回到中国发布，它解析时会当成东 8 区的 21 点来解析，然后使用 `moment("2019-01-01 21:00").tz("Asia/Tokyo").format()` 时进行转换，将东 8 区（中国）的 21 点转换成东 9 区（日本）的 22 点。然而实际上我觉得更常见的是一个人在国外写了一篇文章，自然标注的是国外时间（谁叫 js-yaml 不支持时区记号呢！），然后用另一个参数注明自己现在的地区，这样回到自己的国家之后，发布时仍然以外国时区解析时间。

很显然解决方案是要改变解析时间的时区，但似乎 JavaScript 并没有什么好的修改时区的办法，使用 `moment.tz(时间，地区)` 似乎可以限定解析的时区，但是直接像下面这样做完全没用。

```javascript
const yaml = require("js-yaml")
const moment = require("moment-timezone")
const d = yaml.safeLoad("2019-01-01 21:00:00")  // -> 2019-01-01T21:00:00.000Z
const newd = new Date(d.getTime() + d.getTimezoneOffset() * 60 * 1000) // -> 2019-01-01T13:00:00.000Z
moment.tz(newd, "Asia/Tokyo").toISOString() // -> 2019-01-01T13:00:00.000Z
moment.tz(newd, "Asia/Tokyo").format("YYYY-MM-DD HH:mm:ss z")  // -> 2019-01-01 22:00:00 JST
```

它仍然是将 21 点当作东 8 区转换为东 9 区的 22 点了，时区完全没变，不然应该是有一小时的变化（从东 8 到 东 9），原因是如果一个时间已经标注了时区（各种合法的格式，比如 `JST`、`CST`、`+0800` 等，单独的 `Z` 在 ISO 标准里就表示 UTC，因此也算标定），Moment 就不会用程序员给定的地区做解析，而是用作输出时的转换（等价于 `moment(时间).tz(地区)`）（这么设计 API 的人你出来我保证不打死你）（我觉得我应该去看一眼那个叫 `You-Dont-Need-Momentjs`）的项目。

解决方案似乎没什么特别优雅的，既然你不肯去掉，那我自己造一个没时区的字符串然后再解析一次呗。

```javascript
const yaml = require("js-yaml")
const moment = require("moment-timezone")
const d = yaml.safeLoad("2019-01-01 21:00:00")  // -> 2019-01-01T21:00:00.000Z
const newd = new Date(d.getTime() + d.getTimezoneOffset() * 60 * 1000) // -> 2019-01-01T13:00:00.000Z
moment.tz(moment(newd).format("YYYY-MM-DD HH:mm:ss"), "Asia/Tokyo").toISOString() // -> 2019-01-01T12:00:00.000Z
moment.tz(moment(newd).format("YYYY-MM-DD HH:mm:ss"), "Asia/Tokyo").format("YYYY-MM-DD HH:mm:ss z") // -> 2019-01-01 21:00:00 JST
```

这样结果就对了。至于接下来你想用什么地区的时间输出无所谓，反正内部的时间已经改变了，不再是你本机的时间。

新年新 Bug：当我在研究为什么我的生成器没有在首页生成这篇新文章半小时之后，我发现问题的源头是我把 `2019-01-01` 写成了 `2018-01-01`。

*AlynxZhou*

**A Coder & Dreamer**
