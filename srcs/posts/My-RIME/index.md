---
title: "可能只适合我自己的 RIME 配置"
layout: post
#comment: true
created: 2019-08-19T15:16:00
updated: 2021-07-23T19:05:05
categories:
  - 计算机
  - 输入法
  - RIME
tags:
  - 输入法
  - RIME
---
# 为什么我要折腾这个

在第 n 次忍受不了 RIME 的奇怪操作逻辑之后，我终于决定彻底教育一下这个不听话的输入法，考虑到已经有 n - 1 次失败的前提，做这个决定并不容易。

<!--more-->

首先说明我是 Linux 用户，使用 ibus-rime 做输入引擎，所以使用什么小狼毫鼠须管或者 fcitx-rime 的如果发现不好用最好别烦我，那些我都没用过。（不好意思因为配置这玩意有点暴躁。）

然后我要吐槽一下 RIME 的文档，完全没有一致的类似 API 手册的东西，而且全繁体加上佛振独特的说话风格读起来真的很累，然后这个项目习惯又是起一大堆奇奇怪怪的名字（虽然我有时候也这样）。假如你想修改一点配置，读了文档里的示例“我懂了，巴拉巴拉”，打开配置一看一堆乱七八糟的就懵了。

然后就是网上“致第一次用 RIME 的你”、“也致第一次用 RIME 的你”，我都看过了，首先他们不用 ibus-rime，其次文章内容毕竟有限，每个人需求不一样，有些我需要的地方他们一笔带过了。

-------

更新（2021-07-23）：另外我是被 felixonmars 惯坏了的 Arch Linux 用户，他已经把各种乱七八糟的 RIME 的配置打包到官方仓库了，所以我不需要和那个什么用来配置 RIME 配置的什么什么东风破打交道。如果你不是 Arch Linux 用户的话，我假设你有能力搞明白那个东风破，**因为我搞不明白**，没办法告诉你怎么用。

-------

RIME 的拼音功能确实很好用（虽然有时候它和我对于词组的想法不太一样），我的需求其实只有几项：

- 对于一些 RIME 默认的中国人几乎都用不到的中文标点直接改成英文标点，这个在书写 Markdown 的时候简直折磨死我了，有几个人会输入全角井号？？？打个标题真的很麻烦。
- 有些符号键在其他输入法里约定俗成的就直接输出特定的中文符号，RIME 的默认操作是弹出选择菜单，完全没这个必要，弹出菜单还需要二次选择而且打断了我的按键操作，需要英文标点时候直接切为英文就好了。
- 关掉 RIME 的中英混输功能，在候选框输入英文字母真的很打断思路，我要输入英文要么就是打代码要么就是打单词，反正都不需要输入法，RIME 来就是给我找麻烦。
- 还有一些奇奇怪怪的 RIME 的键位设置，偏偏要和其他输入法不一样，我已经习惯了那些操作，RIME 的键位只会降低输入速度。

下面介绍一下我的配置。当然如果你嫌麻烦，最后我会加上我的配置的 GitHub Repo。

首先建立一个干净的 RIME 配置环境，直接移走 `~/.config/ibus/rime` 然后执行 `ibus-daemon -rdx` 重新生成（就是它文档里扯的部署部署部署）一套配置，由于我用的都是内置输入法所以也不需要什么乱七八糟的东风破 RIME Kit 地球拼音之类的。

然后你进去 `~/.config/ibus/rime` 新版大概有以下几个东西：

- 目录 `build`：~~里面放了各种 RIME 的默认配置，我们不需要动这个。~~ 很好，我搞错了，原来它是从 `/usr/share/rime-data/` 和 `~/.config/ibus/rime/` 下面加载不带 custom 的文件，然后再读取 custom 文件给之前的文件打 patch，最后生成到 `build` 目录下面。
- 目录 `luna_pinyin.userdb`：看起来像是朙月拼音的词库，当然也不用修改。
- 文件 `installation.yaml`：我猜不用管。
- 文件 `user.yaml`：我猜也不用管。

# 各种乱七八糟操作逻辑的配置

按照 RIME 打 patch 的配置方式我们需要在这个目录下创建一个叫 `default.custom.yaml` 的文件，这样就可以给 `/usr/share/rime-data/default.yaml` 这个文件 patch 辣，当然你得先会写 YAML。第一行首先写个叫 `patch:` 的 key，RIME 要求这样，所有的自定义配置都是在 `patch` 字段下面。

怎么确定要修改的 key 名字呢？~~我这里的都是在 `build/default.yaml` 下面找到的，~~ 因为 patch 的是 `/usr/share/rime-data/default.yaml` 所以就去看这个辣，之前又写错了，那个其实是生成的文件。你也可以试试其他的 YAML 文件。

RIME 的文档说什么要用 `/` 把不同层次的 key 折叠成一个比如 `ascii_composer/switch_key`，~~亲测无所谓，我就爱展开了写完整的 YAML，这样更规范。~~ 我说佛老师对不起对不起，我不懂规矩。a/b/c 是只 patch c，展开了则变成了 patch a。

然后首先第一步我要修改输入法列表，我只用朙月拼音简化字模式就行了：

```yaml
patch:
  schema_list:
    - schema: "luna_pinyin_simp"
```

然后就是改掉那个自作聪明的英文输入模式：

```yaml
  # 按 CapsLock 输出大写英文字母。
  ascii_composer/good_old_caps_lock: true
  # `inline_ascii` 在输入框内插入英文。
  # `commit_text` 候选文字上屏并切换至英文。
  # `commit_code` 输入拼音上屏并切换至英文。
  # `clear` 清除拼音并切换至英文。
  # `noop` 屏蔽此按键。
  # 如果你设置 `Caps_Lock` 为 `noop`，
  # 一个奇怪的问题是退格键不能用了，除非取消大写锁定。
  # 所以我直接设置文字上屏了。
  ascii_composer/switch_key/Caps_Lock: "commit_text"
  ascii_composer/switch_key/Shift_L: "commit_code"
  ascii_composer/switch_key/Shift_R: "commit_code"
```

大部分坑我都写在注释里了可以自己看。

然后我看那个设置选单也不是很爽，我习惯简体字，这个也可以自己改：

```yaml
  # 改掉原来的繁体字标题。
  switcher/caption: "【设置菜单】"
  # 用半角斜线而不是奇丑无比的全角斜线做分隔符。
  switcher/option_list_separator: "/"
  # 屏蔽 Ctrl-s 开启菜单，只允许 Ctrl-` 和 F4。
  switcher/hotkeys:
    # - "Control+s"
    - "Control+grave"
    - "F4"
```

然后就是改掉它奇怪的键位，Emacs 键位挺好的，但是有几个不知道为什么用不了，再者就是为什么按向左是跳一个字拼音向右是跳一个字母？

```yaml
# 这里修改的是整个输入法全局的键位，某些输入方案有自己的键位可以单独覆盖。
# 但我暂时不需要。
# Emacs 键位，我喜欢。
# 可是谁给我解释一下为什么 Left 是按字拼音跳而 Right 是按字母跳？
key_binder/bindings:
  - accept: "Control+p"
    send: "Up"
    when: "composing"
  - accept: "Control+n"
    send: "Down"
    when: "composing"
  - accept: "Control+b"
    send: "Left"
    when: "composing"
  - accept: "Control+f"
    send: "Right"
    when: "composing"
  - accept: "Alt+b"
    send: "Shift+Left"
    when: "composing"
  - accept: "Alt+f"
    send: "Shift+Right"
    when: "composing"
  - accept: "Control+a"
    send: "Home"
    when: "composing"
  - accept: "Control+e"
    send: "End"
    when: "composing"
  - accept: "Control+d"
    send: "Delete"
    when: "composing"
  # 这个用不了，不过估计也用不到。
  # - accept: "Control+k"
  #   send: "Shift+Delete"
  #   when: "composing"
  - accept: "Control+h"
    send: "BackSpace"
    when: "composing"
  - accept: "Alt+h"
    send: "Shift+BackSpace"
    when: "composing"
  - accept: "Control+g"
    send: "Escape"
    when: "composing"
  - accept: "Control+bracketleft"
    send: "Escape"
    when: "composing"
  - accept: "Alt+v"
    send: "Page_Up"
    when: "composing"
  - accept: "Control+v"
    send: "Page_Down"
    when: "composing"
```

还没完，我觉得正常人不会用 Tab 在拼音之间切换，除非你一次输入一句话（那你不觉得候选框太小了看着累吗？？？），设置 Tab 为跳候选词更自然一点，但我也不知道为什么 Shift-Tab 用不了：

```yaml
      # 正常人不会用 Tab 切换拼音光标的，相信我。用它切换选项更快。
      # - accept: "ISO_Left_Tab"
      #   send: "Shift+Left"
      #   when: "composing"
      # - accept: "Shift+Tab"
      #   send: "Shift+Left"
      #   when: "composing"
      # - accept: "Tab"
      #   send: "Shift+Right"
      #   when: "composing"
      - accept: "Tab"
        send: "Down"
        when: "has_menu"
      - accept: "ISO_Left_Tab"
        send: "Up"
        when: "has_menu"
      # 鬼知道为什么这个也用不了！
      - accept: "Shift+Tab"
        send: "Up"
        when: "has_menu"
```

以及我觉得正常人不用逗号和句号翻页，毕竟下面的默认设置是逗号句号直接上屏，你设置了翻页也没啥卵用，反正我用减号等号或者上下，不过方括号也不错就是了：

```yaml
  # 这里修改的是整个输入法全局的键位，某些输入方案有自己的键位可以单独覆盖。
  # 但我暂时不需要。
  # Emacs 键位，我喜欢。
  # 可是谁给我解释一下为什么 Left 是按字拼音跳而 Right 是按字母跳？
  key_binder/bindings:
    - accept: "Control+p"
      send: "Up"
      when: "composing"
    - accept: "Control+n"
      send: "Down"
      when: "composing"
    - accept: "Control+b"
      send: "Left"
      when: "composing"
    - accept: "Control+f"
      send: "Right"
      when: "composing"
    - accept: "Alt+b"
      send: "Shift+Left"
      when: "composing"
    - accept: "Alt+f"
      send: "Shift+Right"
      when: "composing"
    - accept: "Control+a"
      send: "Home"
      when: "composing"
    - accept: "Control+e"
      send: "End"
      when: "composing"
    - accept: "Control+d"
      send: "Delete"
      when: "composing"
    # 这个用不了，不过估计也用不到。
    # - accept: "Control+k"
    #   send: "Shift+Delete"
    #   when: "composing"
    - accept: "Control+h"
      send: "BackSpace"
      when: "composing"
    - accept: "Alt+h"
      send: "Shift+BackSpace"
      when: "composing"
    - accept: "Control+g"
      send: "Escape"
      when: "composing"
    - accept: "Control+bracketleft"
      send: "Escape"
      when: "composing"
    - accept: "Alt+v"
      send: "Page_Up"
      when: "composing"
    - accept: "Control+v"
      send: "Page_Down"
      when: "composing"
    # 正常人不会用 Tab 切换拼音光标的，相信我。用它切换选项更快。
    # - accept: "ISO_Left_Tab"
    #   send: "Shift+Left"
    #   when: "composing"
    # - accept: "Shift+Tab"
    #   send: "Shift+Left"
    #   when: "composing"
    # - accept: "Tab"
    #   send: "Shift+Right"
    #   when: "composing"
    - accept: "Tab"
      send: "Down"
      when: "has_menu"
    - accept: "ISO_Left_Tab"
      send: "Up"
      when: "has_menu"
    # 鬼知道为什么这个也用不了！
    - accept: "Shift+Tab"
      send: "Up"
      when: "has_menu"
    - accept: "minus"
      send: "Page_Up"
      when: "has_menu"
    - accept: "equal"
      send: "Page_Down"
      when: "has_menu"
    - accept: "bracketleft"
      send: "Page_Up"
      when: "has_menu"
    - accept: "bracketright"
      send: "Page_Down"
      when: "has_menu"
    # 我觉得正常人不应该用逗号和句号翻页。
    # - accept: "comma"
    #   send: "Page_Up"
    #   when: "paging"
    # - accept: "period"
    #   send: "Page_Down"
    #   when: "has_menu"
```

最后就是那一堆乱七八糟的快捷键了，鬼才记得住，有那时间直接翻菜单就行了，那个 Shift+Space 就是我动不动就变成全角的罪魁祸首，全部不要：

```yaml
    # 鬼才记得住这么多乱七八糟的快捷键，我翻菜单比背这玩意快多了。
    # - accept: "Control+Shift+1"
    #   select: ".next"
    #   when: "always"
    # - accept: "Control+Shift+2"
    #   toggle: "ascii_mode"
    #   when: "always"
    # - accept: "Control+Shift+3"
    #   toggle: "full_shape"
    #   when: "always"
    # - accept: "Control+Shift+4"
    #   toggle: simplification
    #   when: "always"
    # - accept: "Control+Shift+5"
    #   toggle: "extended_charset"
    #   when: "always"
    # - accept: "Control+Shift+exclam"
    #   select: "".next"
    #   when: "always"
    # - accept: "Control+Shift+at"
    #   toggle: "ascii_mode"
    #   when: "always"
    # - accept: "Control+Shift+numbersign"
    #   toggle: "full_shape"
    #   when: "always"
    # - accept: "Control+Shift+dollar"
    #   toggle: "simplification"
    #   when: "always"
    # - accept: "Control+Shift+percent"
    #   toggle: "extended_charset"
    #   when: "always"
    # 你就是那个经常害我变成全角的罪魁祸首！
    # - accept: "Shift+space"
    #   toggle: "full_shape"
    #   when: "always"
    # - accept: "Control+period"
    #   toggle: "ascii_punct"
    #   when: "always"
```

我自己是不习惯写 inline 的字典和列表，都写的展开的。

然后是符号设置了，一开始我以为改 default 里面的符号表就行了，但是最近（2021-03-01）发现不行了，翻了一下代码，发现朙月拼音现在不读 default 了，而是加载 symbols 里面的。然后文档建议的是在输入法方案里面添加自定义的符号表，所以需要建立 `luna_pinyin_simp.custom.yaml` 并修改。

因为我们上面取消了逗号句号翻页，所以这里也就不用显式写 commit 直接上屏了。然后我去掉了一大堆菜单，我输入井号星号波浪线百分号就是想要英文标点，你给我弹个菜单我还得多确认好麻烦的。以及我觉得真的没人用那个巨长的全角斜杠，输入斜杠就是为了斜杠，什么通过朙月拼音命令输入假名有意义吗？我为什么不直接切日语输入法呢？另一些标点在中文语境下直接输出中文标点就好了，需要英文标点时候我敲一下 Shift 比看菜单选容易多了！比如书名号，竖线输出人名中间的点，反斜杠输出顿号之类的。

由于我不是金融行业的，我就把一些英文标点常见但对应中文标点也可能会用到的都丢到了 `$` 的菜单里面：

```yaml
patch:
  # 现在朙月拼音加载标点候选是加载 symbols 里面的，根本不加载 default。
  # 然后文档建议的是自己修改的标点符号表放在输入法方案配置里面。
  # 为了方便编写 Markdown，把一些奇怪的写中文根本用不到的符号弹出菜单改成直接输出英文符号。
  # 另一些直接默认输出中文符号，需要英文符号可以切换英文输入。
  # 有关 `"!": {commit: "！"}` 的写法含义是你设置这个键为翻页按键了，
  # 但是你又想在输入拼音出现选单之后输入这个按键直接上屏（常见的逗号句号问号叹号），
  # 我觉得这是多此一举，为什么你非要拿这几个符号翻页？反正我不用。
  # 乱七八糟的符号都塞给 `$` 就好了反正我不是会计不用天天输入 `￥`。
  # 我不会使用全角英文的，我觉得其他程序员也不会。
  # 但是中文的标点又是全角的，所以我就只改半角。
  punctuator/half_shape:
    "!": "！"
    "\"":
      pair:
        - "“"
        - "”"
    "#": "#"
    "$":
      - "￥"
      - "$"
      - "€"
      - "～"
      - "×"
      - "÷"
      - "°"
      - "℃"
      - "‰"
      - "‱"
      - "℉"
      - "©"
      - "®"
    "%": "%"
    "&": "&"
    "'":
      pair:
        - "‘"
        - "’"
    "*": "*"
    "+": "+"
    ",": "，"
    "-": "-"
    ".": "。"
    "/": "/"
    "\\": "、"
    ":": "："
    ";": "；"
    "=": "="
    "?": "？"
    "@": "@"
    "(": "（"
    ")": "）"
    "[": "【"
    "]": "】"
    "{": "「"
    "}": "」"
    "<": "《"
    ">": "》"
    "^": "……"
    "_": "——"
    "`": "`"
    "|": "·"
    "~": "~"
```

最近的朙月拼音添加了反查笔画的功能，按下反引号并输入候选字来启动，但是反引号对于写 Markdown 的人很常用，所以我要关掉这个恼人的功能：

```yaml
  # 反查占据了宝贵的反引号，导致 Markdown 用户非常痛苦，所以关掉。
  recognizer/patterns/reverse_lookup:
```

总之写完这些配置 **之后要手动移除 `~/.config/ibus/rime/build/` 这个生成目录** 再执行 `ibus-daemon -rdx` 就可以应用了，现在 RIME 用起来就更让我愉快了，接下来就是慢慢养词库就行了。

# 有关为什么 ibus-rime 总是竖着的

ibus-rime 是读取 rime 配置而不是 ibus 配置来设置横竖这一点本身就很离谱了，然后由于 bug 啦其他奇奇怪怪的原因啦好像很难搞清楚，我最近终于搞清楚啦！其实也不是很麻烦。

ibus-rime 会读一个叫做 `ibus_rime.yaml` 的配置文件，有这么一个配置可以让他变成横着的：

```yaml
style:
  horizontal: true
```

可能看了之前的你会和我一样想那就打个 patch 到 `ibus_rime.custom.yaml` 不就行了嘛！但是不行，为什么呢？因为不管是 rime 还是 librime 还是 ibus-rime 都没有提供 `/usr/share/rime-data/ibus_rime.yaml` 的文件，所以你的 patch 找不到被打的文件，那就不会被生成到 `build` 目录里。

不要忘了之前说 rime 会读取 `~/.config/ibus/rime/` 下面的 yaml，所以其实只要自己建立 `~/.config/ibus/rime/ibus_rime.yaml` 写入那段配置就可以啦，因为本来也没有所以就不用打 patch 了，或者你在那两个位置建立一个空的 `ibus_rime.yaml` 然后再打 patch 也行……

-------

更新（2021 年 1 月 26 日）：Arch 的 librime 现在打包了一个 `/usr/share/rime/ibus_rime.yaml` 文件，所以上面手动创建一个 `ibus_rime.yaml` 的办法会被覆盖，所以现在建议创建 `~/.config/ibus/rime/ibus_rime.custom.yaml` 然后对照着打patch，比如我写的是：

```yaml
patch:
  # 舒服不如倒着。
  style/horizontal: true
  # 有些软件的行内预测支持有 bug，所以我一般不开。
  style/inline_preedit: false
```

-------

不要忘了删掉 `build` 目录再 `ibus-daemon -rdx`。

-------

更新（2021-07-23）：最近研究了一下如何扩展 RIME 的词库，发现还是稍微复杂的，我尝试导入了肥猫打包的 `rime-pinyin-zhwiki`。如果你要给某个输入法导入词库，首先你得自己创建一个扩展词库文件让他继承这个输入法本来的词库和你想要的词库，因为输入法配置里面只能指定一个词库配置文件。

所以对于朙月拼音简化字版本，先创建一个叫 `luna_pinyin_simp.extended.dict.yaml`，`.dict.yaml` 之前的名字其实是随便取的，内容如下：

```yaml
# 原来要结合默认词库和第三方词库，
# 需要自己编写一个词库让它 fallback 到朙月拼音和第三方词库。
# 我说佛老师对不起对不起，我不懂规矩。
---
name: luna_pinyin_simp.extended
version: "0.1"
# `by_weight`（按词频高低排序）或 `original`（保持原码表中的顺序）。
sort: by_weight
# 因为导入的朙月拼音词库是繁转简，所以这里不能导入简化字八股文。
# 导入简化字八股文。
# vocabulary: essay-zh-hans
# 选择是否导入预设词汇表【八股文】。
use_preset_vocabulary: true

import_tables:
  - luna_pinyin
  - zhwiki
```

应该很容易懂，我就不多唠叨了，记得里面名字和外面文件名要一致。

然后在 `luna_pinyin_simp.custom.yaml` 的 patch 里面加一行：

```yaml
  translator/dictionary: luna_pinyin_simp.extended
```

我还研究了一下如何添加 emoji 功能，也是靠肥猫打的 `rime-emoji` 包，只要在 `luna_pinyin_simp.custom.yaml` 的 patch 里面加一行：

```yaml
  __include: emoji_suggestion:/patch
```

如果你用的不是 Arch，可能需要自己复制 patch 文件内容而不是简单地使用 include，参见[官方说明](https://github.com/rime/rime-emoji/blob/master/README.md#%E8%87%AA%E5%8A%A9%E5%AE%89%E8%A3%9D)。

当然不要忘了安装这两个依赖的包，我这个配置在 Arch Linux 下面一共需要下面几个包：

```
# pacman -S librime ibus-rime rime-luna-pinyin rime-emoji rime-pinyin-zhwiki
```

当然你要有配置好能显示的 emoji 字体。这也是个坑，等我有时间写一下我的 `/etc/fonts/local.conf` 吧。

-------

# 下载

更新（2021-07-23）：因为加了词库和 emoji 之后文件变多了，请直接去 [GitHub Repo](https://github.com/AlynxZhou/alynx-rime-config/) 获取配置。


