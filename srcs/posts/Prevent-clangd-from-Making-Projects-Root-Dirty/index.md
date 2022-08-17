---
title: 阻止 clangd 污染项目根目录的一些方法
layout: post
#comment: true
created: 2022-07-06 12:19:32
updated: 2022-08-17T16:28:58
categories:
  - 计算机
  - 编程
tags:
  - 计算机
  - 编程
---
Emacs 的 lsp-mode 推荐使用 clangd 分析 C/C++ 代码，用起来体验还不错，但是让人非常恼火的是用户要主动或者被迫地在项目根目录下面添加一些文件，比如 `.clang_complete` 或者 `compile_commands.json` 来让 clangd 知道项目需要包含哪些库的头文件，以及 clangd 会直接把建立的索引丢到项目根目录下面的 `.cache` 目录里。虽然可以把这些加入 `.gitignore`，但保不齐哪个脾气古怪的上游维护者会和你纠缠半天让你解释为什么要加这些，实在是很麻烦。

<!--more-->

`compile_commands.json` 比较好解决，clangd 提供了一个参数 `--compile-commands-dir=<string>` 可以指定查找这个文件的目录，我直接把它设置为 `./build/`，因为大部分项目的 `.gitignore` 都会包含构建目录，也免得运行 `bear -- meson compile` 之后再把这个文件从 `build` 目录移出来。

```elisp
(use-package lsp-mode
  :ensure t
  :commands lsp
  :hook ((c-mode . lsp-deferred)
         (c++-mode . lsp-deferred)
         (c-or-c++-mode . lsp-deferred)
         (lsp-mode . lsp-enable-which-key-integration))
  :bind (:map lsp-mode-map
              ("M-." . lsp-find-definition)
              ("M-," . lsp-find-references))
  :custom
  ;; Move lsp files into local dir.
  (lsp-server-install-dir (locate-user-emacs-file ".local/lsp/"))
  (lsp-session-file (locate-user-emacs-file ".local/lsp-session"))
  (lsp-keymap-prefix "C-c l")
  ;; Only enable log for debug.
  ;; This controls `*lsp-log*` buffer.
  (lsp-log-io nil)
  ;; JavaScript (ts-ls) settings.
  ;; OMG, the FUCKING EVIL SHITTY VSCode TypeScript language server generates
  ;; log in project dir, can MicroSoft stop to let their software put shit in
  ;; front of users?
  (lsp-clients-typescript-server-args '("--stdio" "--tsserver-log-file" "/tmp/tsserver-log.txt"))
  (lsp-javascript-format-insert-space-after-opening-and-before-closing-nonempty-braces nil)
  ;; Always let clangd look for compile_commands.json under build dir so it will
  ;; not make project root dirty.
  (lsp-clients-clangd-args ("--header-insertion-decorators=0" "--compile-commands-dir=./build/" "--enable-config")))
```

对于 `.cache/` 就不是那么好解决了，根据 <https://github.com/clangd/clangd/issues/341#issuecomment-1003560792>，似乎他们并没有关闭或者修改缓存目录的支持。不过我想到一个弯道超车的方案，`git` 本身应该是有从其它位置加载用户定义的 `gitignore` 文件的功能的，我利用这个写一个本地的 gitignore 不就行了吗，搜索之后得到 <https://stackoverflow.com/questions/5724455/can-i-make-a-user-specific-gitignore-file>，操作起来也很简单。首先我把这个文件放到 `~/.config/git/gitignore`，里面写上要忽略的 glob，然后运行 `git config --global core.excludesfile ~/.cache/git/gitignore` 就大功告成。

不过就在我写这篇文章时，clangd 的 issue 上有人回复我，根据 <https://github.com/clangd/clangd/issues/184#issuecomment-998244415>，现在 clangd 应该是会把索引放在 `compile_commands.json` 所在的目录，所以多少也算是解决了问题吧。虽然这样删掉构建目录之后索引缓存也没了，不过我觉得比起重建缓存，还是弄脏项目目录更恶心一点。

更新（2022-08-17）：还有一个头疼的问题是 GLib 的 `g_clear_pointer` 宏里面使用到了对指针本体取 `sizeof` 的语法，而 clangd 默认会认为这是个错误，于是 lsp 就会标出一大堆问题。可以对项目进行设置关掉这一条，不过又会弄脏项目目录，查询文档得知 clangd 会读取 `~/.config/clangd/config.yaml` 这个用户级别的配置文件，于是在里面写入内容关掉这条检查：

```
Diagnostics:
  ClangTidy:
    Remove: bugprone-sizeof-expression
```

然后给 clangd 传递 `--enable-config` 这个参数即可。
