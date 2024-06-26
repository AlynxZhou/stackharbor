---
title: "可能只适合我自己的 RIME 配置 2"
layout: post
#comment: true
created: 2022-07-18T18:50:50
categories:
  - 计算机
  - 输入法
  - RIME
tags:
  - 输入法
  - RIME
---
上一篇：[可能只适合我自己的 RIME 配置](../My-RIME/)

这一篇的原因是我最近在偶然间刷博客刷到一篇 [讲 RIME 简体输入方案的文章](https://blog.coelacanthus.moe/posts/tech/a-new-rime-simp-pinyin-schema/)，里面提到说朙月拼音因为是繁体转简体所以会出现各种错误（其实我个人倒是没怎么遇到过），然后推荐了一个完全针对简体字的输入方案 [极光拼音](https://github.com/hosxy/rime-aurora-pinyin/)，我自己其实只会输入简体字，不怎么需要输入繁体字的功能，所以打算试试。

<!--more-->

已经有人在 AUR 打包了 `rime-aurora-pinyin`，所以我直接拿来用了，然后类似于我上一篇文章处理朙月拼音的办法，给这个也做了一些自定义设置，主要是添加 emoji，修改默认的全角标点上屏行为，以及加载扩展过的字典，不过遇到了几个问题。

首先是我像上篇文章说的那样直接在 patch 下面添加 `__include: emoji_suggestion:/patch` 并不能输入 emoji，我研究了很长时间，甚至以为 emoji 功能依赖繁体转简体。结果其实并不是，打开 `emoji_suggestion.yaml` 可以看到下面几句：

```yaml
patch:
  switches/@next:
    name: emoji_suggestion
    reset: 1
    states: [ "🈚️️\uFE0E", "🈶️️\uFE0F" ]
  'engine/filters/@before 0':
    simplifier@emoji_suggestion
  emoji_suggestion:
    opencc_config: emoji.json
    option_name: emoji_suggestion
    tips: all
```

`switches` 的部分可以先忽略，关键在于 `engine`，这个 emoji 输入的原理是添加一个 filter，它接收一个输入，然后去附带的 opencc 的词典里查找这个输入得到对应的结果，再把这个输出给下一个 filter，按照词典，输入应该是中文字或者词，并且我看了一下词典，简体和繁体是都有的，所以也不存在简繁转换的问题。其实问题在于这段配置会把它作为第一个 filter 加入列表，而极光拼音的默认 filter 列表是这样的：

```yaml
  filters:
    - uniquifier
    - charset_filter@gb2312
    - charset_filter@gbk
```

也就是说如果把 emoji 的 filter 加到第一个，它的输出就要继续经过 `uniquifier`，`charset_filter@gb2312`，`charset_filter@gbk`，后两个是极光拼音为了排除掉几乎用不到的生僻字而添加的。而 emoji 显然不属于 `gb2312` 也不属于 `gbk`，自然就被过滤掉了。

所以我的解决方案是把 emoji 的 filter 加到列表最后，其实加到哪里无所谓，只要你确定前一个 filter 的输出是中文，能触发 emoji 的 opencc 词典就好了，我单独写了一个 `emoji_suggestion.patch.yaml` 文件：

```yaml
switches/@next:
  name: emoji_suggestion
  reset: 1
  states: [ "🈚️️\uFE0E", "🈶️️\uFE0F" ]
engine/filters/@next: simplifier@emoji_suggestion
emoji_suggestion:
  opencc_config: emoji.json
  option_name: emoji_suggestion
  tips: all
```

导入的时候就写 `__include: emoji_suggestion.patch:/`。不过虽然我这个不再需要原来的那个 YAML 了，还是需要 `rime-emoji` 这个项目里其余的文件的。

顺便这也解释了为什么使用朙月拼音时候 emoji 后面的提示框显示的是繁体而非简体，因为朙月拼音从词库直接吐出来的是繁体，然后直接经过第一个 filter 就是 emoji，自然 emoji 查找时候用的就是繁体，然后才会经过简繁转换的 filter，所以如果把 emoji 的 filter 挪到简繁转换的 filter 后面，提示就会变成简体。

解决了 emoji 问题之后还有另一个问题，因为这个 emoji 的 filter 的输入是中文词组，也就意味着必须词库能吐出对应的中文词才能输入 emoji，比如说刚配置出来极光拼音的时候是吐不出来“笑哭”这个词的，所以就不会触发笑哭的 emoji。据说其它平台的输入法也有这个问题。其实没什么太好的解决方案，你可以说自己先手动打几次对应的词然后等 RIME 记住这个输入，不过我觉得也不太好。我想到的办法是既然需要词库里有，不如就让我用 emoji 的 opencc 词典生成一个 RIME 词库，然后扩展词库的时候加进去，这样无论如何都能吐出来了。其实也不是很麻烦，但是需要你把文字转成对应的拼音，那当然不能人工做这个操作了，我利用 Node 的 pinyin 库写了个脚本来做这件事：

```javascript
#!/usr/bin/env node

const fs = require("fs");
const OpenCC = require("opencc");
const {pinyin} = require("pinyin");

// 我的词库只需要简体中文，如果你需要繁体中文，把 `t2s` 改成 `s2t` 应该就好了。
const converter = new OpenCC("t2s.json");

const outputFileName = "emoji_suggestion.dict.yaml";

const inputFileNames = [];

if (process.argv.length <= 2) {
  console.log(`Usage: ${process.argv[1]} file1 file2 ...`);
  process.exit(0);
}

for (let i = 2; i < process.argv.length; ++i) {
  inputFileNames.push(process.argv[i]);
}

const results = {};

for (const inputFileName of inputFileNames) {
  const words = fs.readFileSync(
    inputFileName, "utf8"
  ).split("\n").filter((line) => {
    return line.length !== 0;
  }).map((line) => {
    return line.split("\t")[0];
  });
  for (const w of words) {
    // rime-emoji 的 opencc 词典同时包含简体中文和繁体中文，但比如极光拼音
    // 这种默认不包含简繁转换的方案多半只想要其中一种，所以使用 opencc 对候选词
    // 进行一次转换。
    const word = converter.convertSync(w);
    if (results[word] != null) {
      continue;
    }
    const py = pinyin(word, {
      "heteronym": true,
      "segment": true,
      "style": "normal"
    }).map((array) => {
      // 有些时候就算利用结巴分词了，这个库仍然会没法判断多音字的读音然后丢出好
      // 几个结果，只取第一个好了。
      return array[0];
    }).join(" ");
    // 遇到处理不了的生僻字这个库会直接丢出原本的字……什么奇怪逻辑，只能判断是不
    // 是字母或空格了。
    if (/^[a-z ]*$/.test(py)) {
      results[word] = py;
    }
  }
}

const outputLines = [
  "# Rime dictionary for emoji",
  "# encoding: utf-8",
  "# Generated by `gen-emoji-dict.js` written by Alynx Zhou",
  "",
  "---",
  "name: emoji_suggestion",
  "version: \"0.1\"",
  "sort: by_weight",
  "...",
  ""
];
for (const k in results) {
  outputLines.push(`${k}\t${results[k]}`);
}
// console.log(outputLines.join("\n"));
fs.writeFileSync(outputFileName, outputLines.join("\n"), "utf8");
```

当然这个脚本不是很完美，比如 pinyin 识别不了的生僻字直接忽略了，不过我觉得它都识别不了，我多半也不会打出来的。然后虽然可以利用 jieba 分词提高多音字的准确性，还是有些不正确的，这些遇到了再手动纠错吧。

最后把这个词库添加进扩充词库：

```yaml
# 原来要结合默认词库和第三方词库，
# 需要自己编写一个词库让它 fallback 到极光拼音和第三方词库。
# 我说佛老师对不起对不起，我不懂规矩。
---
name: aurora_pinyin.extended
version: "0.1"
# `by_weight`（按词频高低排序）或 `original`（保持原码表中的顺序）。
sort: by_weight
# 听说默认简化字八股文效果不好，还是算了。
# https://blog.coelacanthus.moe/posts/tech/a-new-rime-simp-pinyin-schema/
# 因为导入的朙月拼音词库是繁转简，所以这里不能导入简化字八股文。
# 导入简化字八股文。
# vocabulary: essay-zh-hans
# 选择是否导入预设词汇表【八股文】。
# use_preset_vocabulary: true

import_tables:
  # 主要是为了肥猫 wiki 词库。极光拼音好像是内置常用简化字表的。
  - zhwiki
  - aurora_pinyin
  - emoji_suggestion
```

顺便说一下我其实也不太了解这个扩展词库的顺序怎么设置比较好，不过我尝试的结果是像这样把 emoji 放在最后面，就不会每次输入在前面提示很多并不常用的 emoji 词组的问题。

我这个脚本生成的词库只有简体，不过我发现朙月拼音的简繁转换还是可以正常处理简体词库的，也就是说会变成 词库出简体 -> 简繁转换 -> 繁体变 emoji，所以直接加给朙月拼音也没问题，如果我需要用繁体中文，可以直接切换方案到朙月拼音（虽然实际上我的配置是简化字版，不过看起来主要区别只是默认是否开启繁体转简体）。平时输入简体则直接用极光拼音。

完整配置在 [GitHub Repo](https://github.com/AlynxZhou/alynx-rime-config/) 更新。
