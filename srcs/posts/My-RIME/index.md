---
title: "可能只适合我自己的 RIME 配置"
layout: post
#comment: true
createdDate: 2019-08-19 15:16:00
categories:
- 计算机
- 输入法
- RIME
tags:
- 输入法
- RIME
---
在第 n 次忍受不了 RIME 的奇怪操作逻辑之后，我终于决定彻底教育一下这个不听话的输入法，考虑到已经有 n - 1 次失败的前提，做这个决定并不容易。

<!--more-->

首先说明我是 Linux 用户，使用 ibus-rime 做输入引擎，所以使用什么小狼毫鼠须管或者 fcitx-rime 的如果发现不好用最好别烦我，那些我都没用过。（不好意思因为配置这玩意有点暴躁。）

然后我要吐槽一下 RIME 的文档，完全没有一致的类似 API 手册的东西，而且全繁体加上佛振独特的说话风格读起来真的很累，然后这个项目习惯又是起一大堆奇奇怪怪的名字（虽然我有时候也这样）。假如你想修改一点配置，读了文档里的示例“我懂了，巴拉巴拉”，打开配置一看一堆乱七八糟的就懵了。

然后就是网上“致第一次用 RIME 的你”、“也致第一次用 RIME 的你”，我都看过了，首先他们不用 ibus-rime，其次文章内容毕竟有限，每个人需求不一样，有些我需要的地方他们一笔带过了。

RIME 的拼音功能确实很好用（虽然有时候它和我对于词组的想法不太一样），我的需求其实只有几项：

- 对于一些 RIME 默认的中国人几乎都用不到的中文标点直接改成英文标点，这个在书写 Markdown 的时候简直折磨死我了，有几个人会输入全角井号？？？打个标题真的很麻烦。
- 有些符号键在其他输入法里约定俗成的就直接输出特定的中文符号，RIME 的默认操作是弹出选择菜单，完全没这个必要，弹出菜单还需要二次选择而且打断了我的按键操作，需要英文标点时候直接切为英文就好了。
- 关掉 RIME 的中英混输功能，在候选框输入英文字母真的很打断思路，我要输入英文要么就是打代码要么就是打单词，反正都不需要输入法，RIME 来就是给我找麻烦。
- 还有一些奇奇怪怪的 RIME 的键位设置，偏偏要和其他输入法不一样，我已经习惯了那些操作，RIME 的键位只会降低输入速度。

下面介绍一下我的配置，只需要一个文件就可以，我没有对朙月拼音的配置做修改，只是修改了默认配置，朙月拼音自己的配置优先级低于默认配置。

首先建立一个干净的 RIME 配置环境，直接移走 `~/.config/ibus/rime` 然后执行 `ibus-daemon -rdx` 重新生成（就是它文档里扯的部署部署部署）一套配置，由于我用的都是内置输入法所以也不需要什么乱七八糟的东风破RIME Kit地球拼音之类的。

然后你进去 `~/.config/ibus/rime` 新版大概有以下几个东西：

- 目录 `build`：里面放了各种 RIME 的默认配置，我们不需要动这个。
- 目录 `luna_pinyin.userdb`：看起来像是朙月拼音的词库，当然也不用修改。
- 文件 `installation.yaml`：我猜不用管。
- 文件 `user.yaml`：我猜也不用管。

按照 RIME 打 patch 的配置方式我们需要在这个目录下创建一个叫 `default.custom.yaml` 的文件，当然你得先会写 YAML。第一行首先写个叫 `patch:` 的 key，RIME 要求这样，所有的自定义配置都是在 `patch` 字段下面。

怎么确定要修改的 key 名字呢？我这里的都是在 `build/default.yaml` 下面找到的，你也可以试试其他的 YAML 文件。

RIME 的文档说什么要用 `/` 把不同层次的 key 折叠成一个比如 `ascii_composer/switch_key`，亲测无所谓，我就爱展开了写完整的 YAML，这样更规范。

然后首先第一步我要修改输入法列表，我只用朙月拼音简化字模式就行了：

```yaml
patch:
  schema_list:
    - schema: "luna_pinyin_simp"
```

然后就是改掉那个自作聪明的英文输入模式：

```yaml
  ascii_composer:
    # 按 CapsLock 输出大写英文字母。
    good_old_caps_lock: true
    # `inline_ascii` 在输入框内插入英文。
    # `commit_text` 候选文字上屏并切换至英文。
    # `commit_code` 输入拼音上屏并切换至英文。
    # `clear` 清除拼音并切换至英文。
    # `noop` 屏蔽此按键。
    switch_key:
      # 如果你设置 `Caps_Lock` 为 `noop`，
      # 一个奇怪的问题是退格键不能用了，除非取消大写锁定。
      # 所以我直接设置文字上屏了。
      Caps_Lock: "commit_text"
      Shift_L: "commit_code"
      Shift_R: "commit_code"
      Control_L: "noop"
      Control_R: "noop"
```

大部分坑我都写在注释里了可以自己看。

然后我看那个设置选单也不是很爽，我习惯简体字，这个也可以自己改：

```yaml
  switcher:
    # 改掉原来的繁体字标题。
    caption: "【设置菜单】"
    # 用半角斜线而不是奇丑无比的全角斜线做分隔符。
    option_list_separator: "/"
    # 屏蔽 Ctrl-s 开启菜单，只允许 Ctrl-` 和 F4。
    hotkeys:
      # - "Control+s"
      - "Control+grave"
      - "F4"
```

然后就是改掉它奇怪的键位，Emacs 键位挺好的，但是有几个不知道为什么用不了，再者就是为什么按向左是跳一个字拼音向右是跳一个字母？

```yaml
  key_binder:
    bindings:
      # Emacs 键位，我喜欢。
      # 可是谁给我解释一下为什么 Left 是按字拼音跳而 Right 是按字母跳？
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

然后是符号设置了，因为我们上面取消了逗号句号翻页，所以这里也就不用显式写 commit 直接上屏了。然后我去掉了一大堆菜单，我输入井号星号波浪线百分号就是想要英文标点，你给我弹个菜单我还得多确认好麻烦的。以及我觉得真的没人用那个巨长的全角斜杠，输入斜杠就是为了斜杠，什么通过朙月拼音命令输入假名有意义吗？我为什么不直接切日语输入法呢？另一些标点在中文语境下直接输出中文标点就好了，需要英文标点时候我敲一下 Shift 比看菜单选容易多了！比如书名号，竖线输出人名中间的点，反斜杠输出顿号之类的。

由于我不是金融行业的，我就把一些英文标点常见但对应中文标点也可能会用到的都丢到了 `$` 的菜单里面：

```yaml
  # 为了方便编写 Markdown，把一些奇怪的写中文根本用不到的符号弹出菜单改成直接输出英文符号。
  # 另一些直接默认输出中文符号，需要英文符号可以切换英文输入。
  # 有关 `"!": {commit: "！"}` 的写法含义是你设置这个键为翻页按键了，
  # 但是你又想在输入拼音出现选单之后输入这个按键直接上屏（常见的逗号句号问号叹号），
  # 我觉得这是多此一举，为什么你非要拿这几个符号翻页？反正我不用。
  # 乱七八糟的符号都塞给 `$` 就好了反正我不是会计不用天天输入 `￥`。
  punctuator:
    # 我不会使用全角标点的，所以只改半角。
    half_shape:
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

总之写完这些配置再执行 `ibus-daemon -rdx` 就可以应用了，现在 RIME 用起来就更让我愉快了，接下来就是慢慢养词库就行了。

最后如果你想试一试，点击 [这里](./default.custom.yaml) 下载。

*Alynx Zhou*

**A Coder & Dreamer**
