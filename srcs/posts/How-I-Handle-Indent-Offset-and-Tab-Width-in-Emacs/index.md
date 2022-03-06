---
title: 我如何在 Emacs 里面处理缩进宽度和 Tab 宽度
layout: post
#comment: true
created: 2022-03-06T22:00:00
categories:
  - 计算机
  - 编程
tags:
  - 计算机
  - 编程
  - Emacs
---
> Tabs are 8 characters, and thus indentations are also 8 characters. There are heretic movements that try to make indentations 4 (or even 2!) characters deep, and that is akin to trying to define the value of PI to be 3.
> -- Linux Kernel Coding Style


当然，我不是在任何情况下都同意上面那句话，虽然在写 C 的时候它是绝对的真理。我个人倾向于取其中一部分——任何试图把 Tab 宽度定义为 8 以外的行为都无异于把 PI 定义为 3。

<!--more-->

写这篇文章是因为我意识到一个问题：Tab 宽度和缩进宽度是两个无关的变量。很多人简单地把缩进宽度理解为 Tab 宽度，不过这不能怪他们，因为大部分的编辑器都把这两者当作相同的东西。而当我重新开始用 Emacs，才发现这两个本来应该是不一样的。

为了防止迷惑，首先解释几个我说的名词：

- Tab 宽度：字面意义上的一个 Tab 字符应该相当于几个空格的宽度，在 Emacs 里面是 `tab-width` 这个变量，我个人建议固定为 8。
- 缩进宽度：当你的代码应该增加一个缩进级别的时候，应该向右缩进几个空格的宽度，在 Emacs 里面每个模式都有不同的变量控制，比如 `c-indent-offset`。
- 缩进级别：代码逻辑层次（或许可以简单理解为作用域）每增加一层，一般就应该增加一个缩进层级。
- 使用 Tab 缩进：不是说只使用 Tab 缩进的意思。事实上如果你的代码可以只使用 Tab 缩进，那么 Tab 宽度为几都无所谓，因为一个缩进宽度就是一个 Tab，但实际上你不一定只使用 Tab 缩进，因为你可能还包含对齐的情况，比方说函数头参数太多，而你希望换行后的参数能够和左括号的下一个字符对齐，那么这个距离大概率无法被 Tab 宽度整除。因此使用 Tab 缩进的含义是“总缩进宽度能够用 Tab 宽度整除的部分用 Tab，余数的部分用空格”，也就是 Emacs 里面 `(indent-tabs-mode 1)` 的效果。
- 使用空格缩进：和上面相反，意味着缩进部分完全不使用 Tab 字符，那么 Tab 宽度为几也无所谓。也就是 Emacs 里面 `(indent-tabs-mode -1)` 的效果。

我个人的建议和倾向主要是两个，第一个是 Tab 宽度固定为 8，第二个则是如果你认为缩进宽度为 8 也就是一个 Tab 对你来说太宽，那么你就应该完全不用 Tab 缩进，也就是使用空格缩进，而不是修改 Tab 宽度。

这样做的理由很简单，主要是我最近修改 GTK 代码时候发现的，GTK 代码是典型的 GNU 风格，我一开始以为它是完全不使用 Tab 缩进，并且缩进宽度是 2，并且我在 Atom 里面也是把这个项目的 Tab 宽度设为 2，但我前段时间发现 GTK 实际上是使用 Tab 的，比如一个很有意思的情况：某段代码的总缩进级别是 5，而它实际上使用的是 1 个 Tab 和 2 个空格，如果你把 Tab 宽度设置成 2，那这段代码在你的编辑器里就会错误的显示成 2 个缩进级别，只有你的 Tab 宽度是 8 的时候才能正确的显示。

这些奇怪缩进的项目给了我两个教训：Tab 宽度和缩进宽度并不是一个东西，以及最好假设 Tab 宽度为 8。当然，以上的建议都是以遵照现有代码的风格为前提，应该不会有人任性到把别人的项目都改成自己风格再提交贡献吧。

我倒不是完全的 Tab 缩进党，比如在 Python 里我设置缩进宽度是 4，而在 JavaScript 里我设置缩进宽度是 2，主要是因为对于 C 这种不允许嵌套函数并且没有类似 class 这种层次的语言来说，缩进级别完全就是函数内部逻辑，当你总缩进宽度达到 24 的时候，你已经在函数里有三层逻辑了。而对 Python 或者 JS 这类函数经常是类的方法的语言，本身函数就已经带着一个缩进级别了，三个缩进级别仅仅只代表两层函数内部逻辑，更别提比如回调函数是匿名函数的情况了。

至于当缩进宽度不是 8 时使用空格缩进而不是用 Tab 然后修改 Tab 宽度的理由也很简单，和上面说的一样，完全使用 Tab 缩进时虽然 Tab 宽度不会影响总缩进级别，但是一旦遇到对齐的情况，Tab 宽度不一致，对齐的部分就不再一样了。

说了这么多理论，该到具体的我怎么在 Emacs 里面处理了，首先是默认值，我个人是定义 Tab 宽度为 8 并且设置允许使用 Tab 缩进。

```elisp
(setq-default tab-width 8)
(setq-default indent-tabs-mode t)
```

默认的 C 编码风格是 GNU，但是 GNU 的编码风格实在是太恐怖了，特别是大部分人的入门书上应该都是 K&R 或者类似的风格。我换成了我喜欢的 linux 内核风格。有的变量你可以直接使用 `setq`，但另一些使用 `setq` 会说你声明了一个新的自由变量，这个时候还是遵照建议使用 Emacs 的 `customize` 系统吧。

```elisp
(customize-set-variable 'c-default-style '((java-mode . "java")
                                           (awk-mode . "awk")
                                           (other . "linux")))
```

前面说过 Emacs 对于不同的模式使用不同的变量作为缩进宽度，这样对于编写配置其实很困难，因为让你时刻记住哪个模式用哪个变量显然不太现实，这里我用了一个比较投机取巧的办法——Emacs 有所谓 buffer-local 变量的设定，也就是说一个变量会有一个默认值，然后每个 buffer 都可以有一个该变量的副本，可以设置成不同的值，如果没有则使用默认值。利用这个功能我创建了一个单独的 buffer-local 变量 `indent-offset`，然后把以上所有这些都设置为该变量的别名，于是我对每个 buffer 只要修改 `indent-offset` 的副本就可以了。我这里只写了我使用到的模式的变量，如果有其他的就加到列表里，或者我看 `doom-modeline` 的代码里几乎包含了所有常见模式的变量，或许可以拿来用。

```elisp
(defconst mode-indent-offsets '(c-basic-offset
                                js-indent-level
                                css-indent-offset
                                sgml-basic-offset
                                python-indent-offset
                                lua-indent-level
                                web-mode-code-indent-offset
                                web-mode-css-indent-offset
                                web-mode-markup-indent-offset
                                markdown-list-indent-width)
  "Different modes' indent variables to make alias to indent-offset.")

(dolist (mode-indent-offset mode-indent-offsets)
  (defvaralias mode-indent-offset 'indent-offset))

(defvar-local indent-offset tab-width)
```

那么接下来就是我们如何针对不同的 buffer 进行不同的设定了，这里分为两部分，一部分是这个 buffer 是否使用 Tab 缩进，另一部分则是缩进宽度设置为多少。Atom 里面有一个状态栏插件，点击就可以快速设置，Emacs 里面我写了两个简单的函数方便调用，执行对应的函数设置是否使用 Tab，并且它们都会询问你想把缩进宽度设置为多少。我分别把这两个函数绑定到 `C-c i TAB` 和 `C-c i SPC`。对于 GTK，操作起来可以是 `M-x indent-tabs RET 2 RET`。

```elisp
(defun indent-tabs (num)
  "Mark this buffer to indent with tabs and set indent offset to NUM chars."
  (interactive `(,(read-number "Indent offset (chars): " indent-offset)))
  (indent-tabs-mode 1)
  (when (/= indent-offset num)
    (setq indent-offset num)))
(global-set-key (kbd "C-c i TAB") 'indent-tabs)

(defun indent-spaces (num)
  "Mark this buffer to indent with spaces and set indent offset to NUM chars."
  (interactive `(,(read-number "Indent offset (chars): " indent-offset)))
  (indent-tabs-mode -1)
  (when (/= indent-offset num)
    (setq indent-offset num)))
(global-set-key (kbd "C-c i SPC") 'indent-spaces)
```

当然以防万一你真的遇到一个脾气古怪的作者，一定要使用 Tab 缩进并修改 Tab 宽度，我也写了个修改 Tab 宽度的函数。这个我绑定到 `C-c i w` 了。

```elisp
(defun set-tab-width (num)
  "Mark this buffer to set tab width to NUM chars."
  (interactive `(,(read-number "Tab width (chars): " tab-width)))
  (when (/= tab-width num)
    (setq tab-width num)))
(global-set-key (kbd "C-c i w") 'set-tab-width)
```

现在我们有了给每个 buffer 修改设定的办法了，但是通常你对某种语言有一个自己偏爱的风格，肯定希望以这个为默认值，所以我写了一些代码，给每个模式设置成我喜欢的默认风格。同样地如果你装了更多的模式，或者和我有不同的喜好，就修改这些列表好了。

这里使用 `(set-tab-width 8)` 是因为有些模式比如 markdown-mode 把 `tab-width` 定义为 4，按照前面说的，我觉得这是错的，同时上游为了保持向前兼容不好修改，于是这里简单地覆盖掉。

```elisp
(defconst indent-tabs-modes '((prog-mode . 8)
                              ;; `markdown-mode` is not a `prog-mode`.
                              (markdown-mode . 8)
                              (gfm-mode . 8))
  "Modes that will use tabs to indent.")

(defconst indent-spaces-modes '((lisp-mode . 2)
                                (emacs-lisp-mode . 2)
                                (js-mode . 2)
                                (css-mode . 2)
                                (html-mode . 2)
                                (yaml-mode . 2)
                                (lua-mode . 3)
                                (python-mode . 4))
  "Modes that will use spaces to indent.")

(dolist (pair indent-tabs-modes)
  (add-hook (intern (concat (symbol-name (car pair)) "-hook"))
            `(lambda () (indent-tabs ,(cdr pair)) (set-tab-width 8))))

(dolist (pair indent-spaces-modes)
    (add-hook (intern (concat (symbol-name (car pair)) "-hook"))
              `(lambda () (indent-spaces ,(cdr pair)) (set-tab-width 8))))
```

有些 modeline 包含一个显示缩进信息的部分，比如 `doom-modeline` 显示 `TAB` 或者 `SPC` 表示使用 Tab 缩进或使用空格缩进，然后如果该模式有自己的缩进宽度变量就显示，没有就显示 Tab 宽度（我没仔细读，总之它对这俩不做显式区分）。而按照上文，我肯定是倾向显式区分这两个东西的，所以我们不用它的，而是自定义一段 modeline，第一部分显示 `TAB` 或 `SPC`，第二段显示 `indent-offset`，第三段显示 `tab-width`。我没太搞懂 Emacs 的 modeline constructor 的语法，我觉得我写对了但却没有，于是最后变成 `:eval` 一个 `format` 调用了。

```elisp
(setq mode-line-misc-info '(:eval (format "%s %d %d"
					  (if indent-tabs-mode "TAB" "SPC")
					  indent-offset
					  tab-width)))
```

最后我在网上抄了两个配置，一个是让它按回车时候不要自动缩进。另一个是修改默认的删除缩进的行为，默认当你在对着一个 Tab 按下退格键的时候，Emacs 把这个 Tab 变成缩进宽度数量的空格，然后删掉一个空格，这太诡异了，我就让它删掉一个字符好了。

```elisp
(setq-default electric-indent-inhibit t)
(setq backward-delete-char-untabify-method nil)
```

这样当你遇到一个和自己习惯不一样的文件，基本只要看情况调用 `indent-tabs` 或者 `indent-spaces` 即可。我在 Atom 用的插件还有一个自动猜测文件是使用 Tab 还是几个空格作为缩进的功能，不过我看了一下代码，它并不能解决我之前说到的 GTK 的问题，也就是说它会猜成 `SPC 2 2` 而不是 `TAB 2 8`，我懒得自己想一个猜测算法，于是就还是靠自己判断了。

一个我比较想实现的功能是记录每个目录我用了什么设定，因为基本上每个项目都用一样的风格，这样就不用每次编辑文件都手动设置。我知道可以在每个目录创建一个文件记录 Emacs 的一些目录范围变量，但是问题是不是所有项目都想让你添加一个编辑器相关的文件，甚至你都不好修改 `.gitignore` 排除你的文件。我比较想把这个存储记录丢到 Emacs 的目录里，不过并不知道怎么实现，如果哪天我搞清楚了，就去写一个。
