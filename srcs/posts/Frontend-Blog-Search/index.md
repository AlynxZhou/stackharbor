---
title: 前端博客搜索引擎
layout: post
#comment: true
created: 2019-09-18 15:24:00
updated: 2020-08-15 18:25:00
categories:
  - 网站
  - ARIA
tags:
  - 网站
  - 搜索
  - JavaScript
  - ARIA
---
本来我的博客有一个前端搜索框，当输入文字时就在侧边栏展开搜索结果，虽然看起来很时髦，但也不能算是什么食用啊不实用的设计方式。而且一开始我觉得既然没有后端处理请求，也就没法单独打开一个专门搜索页面了。

但是这个设计引发了 [@依云](https://blog.lilydjwg.me/) 的吐槽，有一个单独的搜索页面并且对每个搜索结果有专门的 URL 看起来还是个挺合理的需求。但第一我不知道怎么实现第二我一直觉得能用就行所以开始并没有改。不过依云给我发了 Python 文档的搜索实现，让我突然想明白其实按照标准的 HTML 和 JavaScript 是可以实现无后端的搜索引擎的，然后就动手实现了一个。

<!--more-->

首先既然要搜索那还是得有个索引或者数据库，比较简单的方案就是把所有文章的标题 URL 和内容丢到一个 json 文件里面，这个功能在 Hexo 里可以使用 `hexo-generator-search` 实现，我也给我的 Hikaru 添加了这个生成器，用来生成 JSON。

首先第一件事是实现搜索跳转页面，这一步只要简单的使用 HTML 表单就能实现，首先将我的搜索框改造为如下格式：

```html
<form action="{{ getPath("search.html") }}" method="get">
  <button type="submit" class="search-submit" aria-label="{{ __("search") }}">
    <i class="fas fa-search"></i>
  </button>
  <input type="search" id="search-input" class="search-input" name="q" results="0" placeholder="{{ __("search") }}" aria-label="{{ __("search") }}">
</form>
```

使用 `button` 而不是 `input` 的原因是我想用我的图标做搜索按钮，反正 `<input type="submit">` 只是个特化的 `button`。理论上来说不放按钮靠回车提交也是 OK，但是觉得这样又会被某些用户批评不友好了……

搜索框就很简单，`name="q"` 表示生成的 query string 里 keywords 的 key 是 `q`，然后按照表单写法会被提交到 `search.html`，理论上来说搜索应该发 GET 请求所以就是 `method="get"`，当然 POST 就实现不了复制链接查看搜索结果了。

这个表单就是标准的 HTML 表单，不需要用 JavaScript 处理。生成的 GET 请求的 URL 类似于 `/search.html?q=xxx`

然后接下来是处理请求了，既然是发送到 `search.html`，对于静态后端肯定是要返回这个页面的，那就得先创建页面，然后对这个页面进行特殊处理，这里我同样利用生成器生成一个 layout 设置成 `search` 的页面，然后就可以单独给它编写模板添加处理部分了。

`search.html` 加载之后是可以通过 `window.location.search` 获取到 query string 的，然后我加载我修改过的 `search.js`（[这里](/js/search.js)），用它处理搜索过程。同时在页面里添加了一个 container 用来放置检索结果。

`search.js` 其实没什么黑科技，毕竟我们的难点就是在无后端情况下处理关键词和数据库，关键词已经用 `window.location.search` 拿到了，解析一下然后只要通过 ajax/fetch 请求数据就行了，封装 ajax 的代码网上到处都是，实在不行用 jQuery 也成。

然后剔除重复关键词主要是为了优化一下性能，接下来我的解决方案就是简单粗暴 `indexOf()`，不要跟我提什么算法什么优化，短平快实现效果，我是个实用主义者，目前这一步其实还没有成为瓶颈。

然后对于原版文件我的改进主要是按匹配次数排序，文章出现关键词越多则排序越靠前，相对可以提高效率。

接下来使用正则表达式给关键词加上 `<strong>`，这样显示起来比较显眼。最后把字符串拼起来显示就好了。其实这里的算法还有点意思，比如假如两个关键词出现的位置中间大于多少个字符则插入省略号，否则合并两个的上下文，具体实现也可以参照代码。

如果有性能瓶颈的话，多半也会先出现在 ajax，不过目前我文章还没有多到加载不出来的情况，也许可以靠分块加载解决？

更新（2020-08-15 18:25:00）：我给代码添加了简单的分块支持。由于这里需要主题和生成器约定好路径，不太适合让生成器自动生成路径，所以采用了一个简单的方法就是让用户在设置文件里手动指定几个 JSON 文件的路径，然后生成器只是读取一下配置，假如是数组就分块写到指定好的路径里面。然后前端查找的时候分别异步查找每个文件并合并排序结果，理论来说大概会有性能提升？

最后我加了个简单的 SpinKit 动画，在查询结束之前先跑一下提升用户体验。

*Alynx Zhou*

**A Coder & Dreamer**
