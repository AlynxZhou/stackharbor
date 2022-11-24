---
title: Fontconfig 和 Noto Color Emoji 和抗锯齿
layout: post
#comment: true
created: 2021-09-08T09:33:33
updated: 2022-11-24T12:39:26
categories:
  - 计算机
  - 字体
tags:
  - 计算机
  - Linux
  - 字体
---
我一直用的是以前积攒的一份 fontconfig 配置，主要功能就是设置对于无衬线字体优先用 Roboto 显示英文字体，然后回退到 Noto Sans CJK SC 显示中文字体，因为 Roboto 比 Noto 的英文字好看，以及对等宽字体优先用 Monaco。虽然大部分都是网上抄来的，我自己并不太懂，但是这个配置一直工作的还可以。直到我开启了 RIME 内置的 emoji 输入法，发现 emoji 显示成了空白。

<!--more-->

一开始没觉得是什么很难解决的问题，在字体列表末尾加上 Noto Color Emoji 不就行了？不过事情要是真的这么简单，也就没必要写个博客记下来了。忘记当时怎么查的了，总之是搜到 [一个 firefox 的 bug](https://bugzilla.mozilla.org/show_bug.cgi?id=1454152)，提 bug 的人表示自己一直是开着 hintfull 和 antialias 的，关掉这个 Noto Color Emoji 才能显示。于是我看了一眼我的配置，也有这么一段：

```xml
<!-- 针对所有字体的默认设置，力求显示效果最好。 -->
<match target="font">
  <!-- 禁用内嵌点阵。 -->
  <edit name="embeddedbitmap" mode="assign">
    <bool>false</bool>
  </edit>
  <!-- 禁用合成粗体。 -->
  <edit name="embolden" mode="assign">
    <bool>false</bool>
  </edit>
  <!-- 设置显示器为 RGB 排列。 -->
  <edit name="rgba" mode="assign">
    <const>rgb</const>
  </edit>
  <!-- 开启轻度微调。 -->
  <edit name="hinting" mode="assign">
    <bool>true</bool>
  </edit>
  <edit name="autohint" mode="assign">
    <bool>true</bool>
  </edit>
  <edit name="hintstyle" mode="assign">
    <const>hintslight</const>
  </edit>
  <!-- 开启抗锯齿。 -->
  <edit name="antialias" mode="assign">
    <bool>true</bool>
  </edit>
</match>
```

那我觉得针对 Noto Color Emoji 关掉这几个选项就行了，正好这个 bug 下面就有人提供了配置：

```xml
<!-- Noto Color Emoji 不支持开启抗锯齿和微调，所以在全局开启之后还得给它关掉。 -->
<match target="font">
  <test name="family" qual="any">
    <string>Noto Color Emoji</string>
  </test>
  <edit name="scalable" mode="assign">
    <bool>true</bool>
  </edit>
  <edit name="embeddedbitmap" mode="assign">
    <bool>true</bool>
  </edit>
  <edit name="hinting" mode="assign">
    <bool>false</bool>
  </edit>
  <edit name="antialias" mode="assign">
    <bool>false</bool>
  </edit>
</match>
```

重新登录，输入法里面的 emoji 都显示出来了，我以为事情到这里就结束了，没想到噩梦才刚刚开始……

某一天用 firefox 打开 twitter，发现里面字体都是毛毛糙糙的，我觉得很离奇，明明我只是对 Noto Color Emoji 关了抗锯齿，为什么 twitter 的字体也受到影响了？而且其他页面的字体都是正常的，后来我又打开过一个微软的文档页面，也出现了类似的毛病。我研究了一下发现这两个页面都用了自定义的 webfont。可是 webfont 和 Noto Color Emoji 有什么关系？人类摸不着头发。

我查了很多博客文章，不过它们都关注在正常的字体顺序上，还没有人研究 webfont 有什么问题。于是我只能自己研究，不过我不能保证我对 fontconfig 的理解完全正确，这东西看起来非常复杂，不过我还是大致搞清楚了一些事情。引用我看到的 [一篇英文博客](https://eev.ee/blog/2015/05/20/i-stared-into-the-fontconfig-and-the-fontconfig-stared-back-at-me/) 的标题：

<div class="centerquote">
当我凝视着 fontconfig 时，fontconfig 也在凝视着我。
</div>

首先我找到了这个叫 [fontconfig 几个常见的坑](http://marguerite.su/posts/fontconfig_%E5%87%A0%E4%B8%AA%E5%B8%B8%E8%A7%81%E7%9A%84%E5%9D%91/) 的文章，里面信息量挺大的，不过说实话没有解决掉我的问题——我明明只对匹配到 Noto Color Emoji 时候关掉了抗锯齿，为什么 webfont 也受到影响了呢？这篇文章主要介绍怎么调整字体列表顺序，不过说实话我没有遇到那么多障碍，简单的写法就工作了。同一个作者还写了 [Color Emoji in openSUSE](http://marguerite.su/posts/color_emoji_in_opensuse/) 的文章，不过总感觉是交待到一半，翻了文章列表没有后续了。而且这篇文章关心的也是字体 fallback 时候某些字符选到错误的字体的问题，和我这个不太相关。

然后还有 [Linux fontconfig 的字体匹配](https://catcat.cc/post/2020-10-31/) 这篇文章，它介绍了浏览器是怎么和 fontconfig 合作匹配字体的，但是对于 webfont 也是没有提到。包括它的后续文章 [用 fontconfig 治理 Linux 中的字体](https://catcat.cc/post/2021-03-07/) 也没提到和 Noto Color Emoji 相关的调整（可能和它用的 Twemoji 有关系）。

然后我尝试按照文章里的办法对 firefox 进行一个 fontconfig 的调试，`FC_DEBUG=4 firefox --new-instance --private-window https://twitter.com/` 直接吐了 12 万行输出，我实在是不知道从哪里看了。翻了一下午胡乱修改了好几次配置也没找出来哪里有问题。就在我眼花的时候偶然看到一条有用的，fontconfig 吐出来一个匹配结果，family 是 `Roboto, Noto Sans CJK SC, Noto Color Emoji, sans-serif…` 等等一长串，style 是 `¶`。这个就很有趣了，因为我看 firefox 里面，twitter 用的 webfont 叫 TwitterChirp，它的 style 也是这么个 `¶`（我怀疑是个 bug），那这条可能就是相关的了。然后我发现这个匹配结果下面 antialias 是 false，但理论上来说这条又不是最先匹配到 Noto Color Emoji，怎么会应用 Noto Color Emoji 的设置呢？我猜测是因为默认 test 语句是 `qual="any"` 含义似乎是只要匹配到的有一个满足就应用，正常匹配一个字体应该是不会返回这么一长串 family 的（我用 `fc-match` 一般只返回一个符合条件的 `family`），虽然我不知道为什么 webfont 会返回这么一长串结果，总之我改成 `qual="all"` 也就是必须所有结果都是 Noto Color Emoji 时候才应用就好了对吧！然后重新登录，emoji 能显示，我以为已经胜利了！结果事情证明是半场开香槟……

当我以为完全没问题，正打算用 atom 整理一下这份配置的时候，突然发现不对劲了……Atom里面的中文字体失去了抗锯齿……你可以想像我当时心情有多绝望。具体为什么还是同样的我不知道，不过我猜测恰好是和 [Linux fontconfig 的字体匹配](https://catcat.cc/post/2020-10-31/) 里面提到的 chromium 查询字体的奇葩逻辑有关系……可能只是它某次查询刚好漏掉了 Noto Sans CJK SC 匹配到了 Noto Color Emoji 然后就给了个 antialias false，然后再通过什么奇怪的 UI 字体查询找出了中文字体……反正总之我的心好累，我实在不知道该怎么解决了，我只想用个 emoji，我甚至还给 Noto Color Emoji 提了请求支持抗锯齿的 issue……然后我决定要不换成 Twemoji 算了。

Twemoji 倒也不是开箱即用，它不能禁用内嵌点阵字体，不过好歹这个不像抗锯齿那么要命，就算影响了其他字体大概也许能够忽略吧……于是我改成了这个。不过这时候我突然灵光一闪有了新主意。

按照前面那几篇博客和 [这一篇手册翻译](http://www.jinbuguo.com/gui/fonts.conf.html) 的讲解，fontconfig match 的 target 分为三个阶段，第一是 scan，扫描字体文件，构建一个集合。然后是 pattern，按照规则构建一个字体列表。最后是 font，意味着已经挑出了需要的字体列表。所以调整字体回退顺序一般都放在 pattern 阶段。而网上大部分都把对于 freetype2 微调的选项放在了 font 阶段，不过我想放在 scan 阶段是不是也可以？这样扫描到 Noto Color Emoji 的时候就对它设置选项，也就不存在 family 列表可能有很多项的问题了。测试了一下发现基本一切正常，于是就把所有 freetype2 微调选项移到了 scan 阶段。（我不是 fontconfig 的专家，这一段要是有错误还希望指正。）

下面是我现在的配置，首先我配置文件放的位置是 `/etc/fonts/local.conf`，因为 `/etc/fonts/fonts.conf` 是发行版默认的设置，用来加载其他配置文件所以不能改，然后我想对所有用户都生效，所以没有放在我的家目录，Arch Linux 包含一个 `/etc/fonts/conf.d/51-local.conf` 文件，`/etc/fonts/fonts.conf` 会加载它，然后它再加载 `/etc/fonts/local.conf`。

首先是 XML 开头必须要有的那些东西：

```xml
<?xml version="1.0"?>
<!DOCTYPE fontconfig SYSTEM "fonts.dtd">
<fontconfig>
```

然后接下来就是针对所有字体的 freetype2 微调选项，按照上面介绍的放在 scan 阶段：

```xml
  <!-- 针对所有字体的默认设置，力求显示效果最好。 -->
  <match target="scan">
    <!-- 禁用内嵌点阵。 -->
    <edit name="embeddedbitmap" mode="assign">
      <bool>false</bool>
    </edit>
    <!-- 禁用合成粗体。 -->
    <edit name="embolden" mode="assign">
      <bool>false</bool>
    </edit>
    <!-- 设置显示器为 RGB 排列。 -->
    <edit name="rgba" mode="assign">
      <const>rgb</const>
    </edit>
    <!-- 开启轻度微调。 -->
    <edit name="hinting" mode="assign">
      <bool>true</bool>
    </edit>
    <edit name="autohint" mode="assign">
      <bool>true</bool>
    </edit>
    <edit name="hintstyle" mode="assign">
      <const>hintslight</const>
    </edit>
    <!-- 开启抗锯齿。 -->
    <edit name="antialias" mode="assign">
      <bool>true</bool>
    </edit>
  </match>
```

内嵌点阵的效果通常没有矢量绘制的效果好，合成粗体也只是个临时方案，微调什么的见仁见智了就。

然后对于 Twemoji 要打开内嵌点阵：

```xml
  <!-- 当然另一个简单方案是换成 Twemoji，不过它不能关这个。 -->
  <match target="scan">
    <test name="family" qual="any">
      <string>Twemoji</string>
    </test>
    <edit name="embeddedbitmap" mode="assign">
      <bool>true</bool>
    </edit>
  </match>
```

对于 Noto Color Emoji 也是打开内嵌点阵关闭微调和抗锯齿：

```xml
  <!-- Noto Color Emoji 不支持开启抗锯齿和微调，所以在全局开启之后还得给它关掉。 -->
  <match target="scan">
    <test name="family" qual="any">
      <string>Noto Color Emoji</string>
    </test>
    <edit name="scalable" mode="assign">
      <bool>true</bool>
    </edit>
    <edit name="embeddedbitmap" mode="assign">
      <bool>true</bool>
    </edit>
    <edit name="hinting" mode="assign">
      <bool>false</bool>
    </edit>
    <edit name="antialias" mode="assign">
      <bool>false</bool>
    </edit>
  </match>
```

我个人还有一个需求，Monaco 作为一个等宽字体竟然支持连字，导致某次我看别人的代码时候 `fi` 连在一起让我以为他没有对齐缩进，结果人家说是我的问题，十分尴尬，于是我在这里关掉了它。不过有些程序比如 firefox 是不支持这个的，解决方法是用 FontForge 编辑字体文件删掉连字相关的数据再导出……

```xml
  <!-- 我真不理解一个等宽字体要连字功能干嘛？故意变得不等宽？ -->
  <match target="scan">
    <test name="family" qual="any">
      <string>Monaco</string>
    </test>
    <edit name="fontfeatures" mode="append">
      <string>liga off</string>
      <string>dlig off</string>
    </edit>
  </match>
```

更新（2022-11-24）：某些网站的前端脑子里不知道装的什么东西，比如简书的前端可能脑子里装的是苹果，他们把 `-apple-system,BlinkMacSystemFont,"Apple Color Emoji"` 排在 `sans-serif` 前边，于是你看到的数字可能是 Emoji 里面的。再比如 B 站直播首页所有的字体都是微软字体，连 `sans-serif` 都没有，你没看错，这个不合格的前端连最后要加 `sans-serif` 保证兼容性都不知道，而且还把 Microsoft Sans Serif 拼错了，如果我是他的老板，我真想一拳打在他头上告诉他这个世界不是只有微软字体，然后再把他开除掉。现在我需要加条规则让他匹配到我想要的字体（虽然这样在一些需要微软雅黑的 office 软件里面可能有问题，不过反正我的工作不需要用垃圾 office 哈哈哈哈哈哈哈哈哈哈），所以需要对这些弱智做一些特殊照顾。因为 Fontconfig 是按顺序处理的，所以如果你想把某个字体换成 `sans-serif` 之类的处理，就得在那之前进行替换。然后很多网站写的都是 Apple Color Emoji，我们这里自然是没有的，要换成我们默认的。

```xml
<!--
    有一点需要注意，sans-serif，serif，monospace, emoji 这些默认字体名并不是真正
    的字体，而是锚点，是用来进行后续的字体插入的。因此如果你想通过把一些字体映射
    成 sans-serif 来修正一些网站奇怪的逻辑的话，请务必写在处理 sans-serif 部分之
    前，这样才会被正常替换。
  -->
  <!--
    我建议这个世界新增一种物种叫苹果人，即打开脑壳之后没有脑子，装的全是苹果的
    人，显然简书的前端就属于此类（-apple-system,BlinkMacSystemFont,
    "Apple Color Emoji","Segoe UI Emoji","Segoe UI Symbol","Segoe UI",
    "PingFang SC","Hiragino Sans GB","Microsoft YaHei","Helvetica Neue",
    Helvetica,Arial,sans-serif）。正常人怎么会把 Apple Color Emoji 放前面？
  -->
  <alias>
    <family>-apple-system</family>
    <prefer>
      <family>sans-serif</family>
    </prefer>
  </alias>

  <alias>
    <family>BlinkMacSystemFont</family>
    <prefer>
      <family>sans-serif</family>
    </prefer>
  </alias>

  <!--
    B 站直播首页的前端认为这个世界上只有微软字体（Arial,Microsoft YaHei,
    "Microsoft Sans Serif",Microsoft SanSerf,微软雅黑），
    所以我不得不开着这几个规则。如果我是他的老板，我就会开除掉这个不合格的前端。
  -->
  <alias>
    <family>Microsoft Sans Serif</family>
    <prefer>
      <family>sans-serif</family>
    </prefer>
  </alias>

  <!-- 我机器上可能真的有微软雅黑，而我真的不想看见它们，反正我也不做排版。 -->
  <alias>
    <family>Microsoft YaHei</family>
    <prefer>
      <family>Roboto</family>
      <family>Noto Sans CJK SC</family>
    </prefer>
  </alias>

  <!-- 替换 Apple Color Emoji 字体。 -->
  <alias>
    <family>Apple Color Emoji</family>
    <prefer>
      <family>emoji</family>
    </prefer>
  </alias>

  <!-- 替换 Noto Color Emoji 字体。 -->
  <!-- <alias> -->
  <!--   <family>Noto Color Emoji</family> -->
  <!--   <prefer> -->
  <!--     <family>emoji</family> -->
  <!--   </prefer> -->
  <!-- </alias> -->

  <!-- 这也是一个常见的默认字体，我的 UI 字体就是无衬线。 -->
  <alias>
    <family>system-ui</family>
    <prefer>
      <family>sans-serif</family>
    </prefer>
  </alias>
```

然后就是常见的默认字体代称（sans-serif，serif，monospace，emoji）回退列表了，我是按照 `Roboto/Roboto Slab/Monaco -> Noto Sans/Serif (Mono) CJK SC -> Noto Color Emoji` 的顺序回退的：

```xml
  <!-- 然后该配置我们喜欢的默认字体了。 -->
  <!-- 默认无衬线字体。 -->
  <alias>
    <family>sans</family>
    <prefer>
      <family>Roboto</family>
      <family>Noto Sans CJK SC</family>
      <family>emoji</family>
    </prefer>
  </alias>

  <alias>
    <family>sans-serif</family>
    <prefer>
      <family>Roboto</family>
      <family>Noto Sans CJK SC</family>
      <family>emoji</family>
    </prefer>
  </alias>

  <!-- 默认有衬线字体。 -->
  <alias>
    <family>serif</family>
    <prefer>
      <family>Roboto Slab</family>
      <family>Noto Serif CJK SC</family>
      <family>emoji</family>
    </prefer>
  </alias>

  <!-- 默认等宽字体。 -->
  <alias>
    <family>monospace</family>
    <prefer>
      <family>Monaco</family>
      <family>Noto Sans Mono CJK SC</family>
      <family>emoji</family>
    </prefer>
  </alias>

  <!-- 默认 emoji 字体。 -->
  <alias>
    <family>emoji</family>
    <prefer>
      <family>Noto Color Emoji</family>
    </prefer>
  </alias>
```

这里的写法和 `<alias>` 是等价的。设置完这个记得把所有软件的自定义字体设置里面无衬线设置为 `sans-serif`，有衬线设置为 `serif`，等宽设置为 `monospace`，这样才会遵守这里的回退顺序。

剩下还有一些冗长的用来在不同语言下选择不同的 Noto Sans CJK 变体的设置，因为 Noto Sans CJK 是一个多语种集合字体，然后中日韩对一些汉字有不同的变体，所以需要这一段，不过我就不贴在这里了，完整的文件可以在文章末尾下载。

最后收个尾：

```xml
</fontconfig>
```

完整的文件在这里：[local.conf](./local.conf)。

😼


