---
title: Emacs 和 Lazy Loading 和 use-package
layout: post
#comment: true
created: 2023-01-20T17:55:00
categories:
  - 计算机
  - Emacs
tags:
  - 计算机
  - Emacs
---
事先叠 buff：我不是说 `use-package` 一定要这么用，我也不是说所用不用 `use-package` 的人都不好，我只是说我觉得应该这样用 `use-package` 比较合适。

`use-package` 是个好东西，因为它解决了 Emacs 插件包从安装到配置的全过程，可以让配置更结构化。不过也有人觉得 `use-package` 关键字过于复杂，总是没办法确定什么配置写到什么字段里面，也不知道展开之后悄悄发生了什么事情。从我再次决定自己打造一份 Emacs 配置以来看了很多不同人的配置，发现他们使用 `use-package` 的方式也是五花八门，有些人不爱用 `:bind` 和 `:hook`，干脆自己在 `:config` 里面调用 `define-key` 和 `add-hook`，有些人不清楚为什么自己的 `:config` 被延迟运行了，干脆全都用 `:init`。还有些人直接换成了其它号称更简单可控的替代品。我不是说上面这些方法都错了，实际上只要能得到想要的结果也无所谓怎么写，但我是一个比较注重逻辑的人，所以研究了一下到底这些关键字是怎么回事，并且试图写篇文章记录我推荐的写法。本来我打算写在注释里的，可是感觉写得太多，所以就放到博客里了。

问题的核心无非是：为什么我写的 `:config` 没有运行？到底什么情况下会有延迟加载？我打算在这里详细分析一下。

首先 Emacs 有一种叫做 autoloads 的东西，插件包的作者可以在某些函数前加上 autoloads 标记，然后创建 autoloads 文件。这个功能的作用很好理解，原本在启动时需要加载所有插件包的文件以便用户使用相应的功能，但不是所有的插件都是启动时就需要，只在启动时加载会让启动速度变得很慢。有了 autoloads 之后启动时只要加载 autoloads 文件，里面定义了如果运行某个函数，就去加载某个文件，这样等到对应的函数第一次运行的时候才被加载，从而提高启动速度。

而 `use-package` 做了什么呢？`use-package` 可以自动创建 autoloads，这样即使一个包本身没有 autoloads，也是可以延迟加载的。最简单的触发这个逻辑的关键字是 `:commands`，就是给后续的函数创建 autoloads 的意思。但如果你仔细阅读文档，就会发现还有几个创建 autoloads 的关键字，分别是 `:bind`、`:hook`、`:mode` 和 `:interpreter`。**如果有这几个关键字，`use-package` 不会立即加载一个包，而是依靠创建的 autoloads 实现延迟加载。**这也很好理解，这几个关键字的意思都是“在某种情况下启用”，所以自动延迟加载也很好理解。

然后为什么会遇到 `:config` 不会运行所以有些人统统把要调用的语句写到 `:init` 里面的问题呢？一般都是发生在类似下面的写法（既有快捷键 / 钩子，又要一启动就启用什么模式）里：


```lisp
(use-package marginalia
  :ensure t
  :bind (:map minibuffer-local-map
              ("M-A" . marginalia-cycle))
  :config
  (marginalia-mode 1))
```

我们得明确 `:init` 和 `:config` 的区别，`:init` 是“无论包有没有加载，都一定会先执行”的配置，`:config` 是“包加载之后才会被执行”的配置。然后按照上面关于延迟加载的分析，这就变成了一个“先有鸡还是先有蛋”的问题：

1. `use-package` 给这个包创建了快捷键对应的 autoloads，于是这个包不会立刻加载。
2. `:config` 里面 `(marginalia-mode 1)` 要等到包加载之后才运行。
3. 这个包本身有给 `marginalia-mode` 创建 autoloads，只要调用 `marginalia-mode` 就会加载这个包，但是根据 1 和 2，这句不会被调用，包也不会被加载，除非按了 1 里面的快捷键。

打破这个循环的办法不止一种，比如把 `(marginalia-mode 1)` 写进 `:init`，这样无论如何都会调用它，于是 `use-package` 创建的 autoloads 被忽略了，包肯定会被加载。但我个人倾向于把这种启用模式的函数放在 `:config` 里面，而且作者也推荐不要在 `:init` 里面放过于复杂的函数，这时候打破循环的办法也很简单，只要使用 `:demand t`，告诉 `use-package` 立即加载这个包即可。

以上的内容其实都写在 <https://github.com/jwiegley/use-package#notes-about-lazy-loading>，只是可能很多人没有注意或者没看明白，于是我试图在这里更详细的解释一下。可能有人会问搞这么复杂真的有意义吗？我直接自己不要延迟加载直接都 `require` 不可以？不过我觉得这种激进的延迟加载方案确实让我的 Emacs 启动非常快，所以大概是有意义的吧。
