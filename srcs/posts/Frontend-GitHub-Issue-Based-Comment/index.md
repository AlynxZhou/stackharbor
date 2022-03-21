---
title: 基于 GitHub Issue 的前端评论框
layout: post
created: 2020-08-15 18:25:00
updated: 2022-03-21 16:46:46
categories:
  - 网站
  - ARIA
tags:
  - 网站
  - 评论
  - JavaScript
  - ARIA
---
# 造轮子是病，得治。

自从造了 [前端博客搜索引擎](../Frontend-Blog-Search/) 的轮子之后，我对自己的能力有了极大的信心，同时也掌握了一些有趣的用法，于是把枪口瞄准了下一个我看着不顺眼的地方——评论框。

（这标题怎么写的和毕业论文似的！）

<!--more-->

# 轮子也不是白造的。

从我建站以来我的评论框就命途多舛，Disqus 虽然是最著名的评论系统，但是在国内访问不太顺畅。多说火了一段时间之后便关门大吉，HyperComments 则在我用了一段时间后发邮件提示要收费了，于是我之前的评论便华丽流失。而对于 Valine 这种基于 LeanCloud 的评论系统，我对 LeanCloud 不甚了解所以也不想尝试（而且 Valine 现在似乎转向闭源了，当初也许是个正确的决定）。然后赶上 Gitment 和 Gitalk 火了起来，大家意识到 GitHub Issue 正是个放评论的好地方。但是由于 [Gitment 和 Gitalk 采用他们自己的服务器实现博客评论框提交——转发到 GitHub API 的实现](https://github.com/iissnan/hexo-theme-next/pull/1634#issuecomment-313596649)，以及 [采用的 OAuth App 权限过高](https://www.v2ex.com/t/535608)，有人怀疑这不太安全，于是我也没太敢参与。后来遇到 [comment.js](https://github.com/wzpan/comment.js) 这个项目，它绕过了提交评论的问题——直接给一个到 GitHub Issue 的评论框的链接就可以了。于是我就开始用这个，至于什么 utterances 这种用 GitHub App 降低权限的评论系统，我也懒得尝试了。

但我为什么决定替换掉 comment.js 我已经记不清楚了，可能是为了对主题的显示有更好的掌握吧，毕竟它带有自己的 CSS 样式而且经常和我的冲突，也可能是因为它迟迟没提供像 Disqus 一样查找每篇文章评论数目的功能，不过它现在已经不再维护了，所以我也算是未雨绸缪。

# 事情本该很简单。

研究了一下原理其实并不是很难，首先就是从 GitHub 的 API 上 ajax 获取数据，然后操作 DOM 添进去就可以了，所以我就先阅读了 [GitHub API 的文档](https://docs.github.com/en/rest/reference/issues)，总之还算容易，只要先获取一个仓库的 issue 列表，然后按照某种方法在里面查找相关的 issue，如果没有就渲染一个到新建 issue 的链接，否则获取该 issue 的全部评论并显示就可以了。

查找 issue 的 key 也算容易，我给它做成了函数参数，然后在主题模板里填一个每个页面唯一的字符串就行了，比如文章标题，然后新生成 issue 页面时候就把这个作为 issue 的标题，这样查找 GitHub Issue 页面时候也比较容易看。然后 GitHub 把 PR 也视为 issue，这个不要紧，收到数组之后过滤一下就好了。而显示数目我琢磨了一下其实也没什么难的嘛！我都获取到所有 issue 的信息了。做一个新的函数，主题作者在页面上放置一系列空的元素并设置好 class 属性，然后同样地把每个页面唯一的字符串设置成这些元素的属性（Disqus 也是这样的），加载函数时候把 class 作为参数传进去，分别从 issue 列表里查找对应的标题就可以了。还有一个奇葩问题是 GitHub issue 的 comments 指的是除了顶楼以外的评论，但很显然看起来不是这样的，这个也简单，直接把顶楼加到数组里就成了嘛。剩下就是艰苦的在浏览器里刀耕火种写 JS 拼 HTML 字符串发 ajax 请求写嵌套回调（没有 async/await 也太痛苦了吧！。

一切正常工作了一段时间一后我发现不太对劲，怎么评论太多的时候显示不全呢？仔细查了一下发现 GitHub API 是强制分页的，也就是说不管你怎么设置，最多一次只能获取 100 条评论，默认则是 30 条，本来我不太想给博客加评论分页功能的，现在看来是 GitHub 被迫让我加啊。当然这个并不是最痛苦的，最痛苦的是它查找仓库的 issue 列表的时候也是强制分页的！这就麻烦了，还记得我们之前说要获取到列表之后查找标题吗？获取不到完整列表还怎么查找啊！

当然你可以说按顺序多查几页不就行了嘛，这就是它分页难受的地方了！ajax 是异步的啊亲！你不会想让我一个 for 循环几个 ajax 还指望优雅的等他们结束了再跑查找吧！我知道能用 `Promise.all()` 解决，但是由于我大发慈悲的支持使用 IE11 的用户（微软我&A%￥S&……省略一千字儒雅随和），所以我的函数是基于回调的，那也没什么办法。而且这样首先你得读一下仓库信息才能知道有多少个 open 的 issue（没错只能算 open 的不能算 close 的！所以在后面查 issue 的时候我也不得不筛选掉 close 的，不过这大概也许是个 feature？），然后自己计算有多少页。最后我只能造了一个不那么优雅的尾递归回调（反正就那个意思），不过它工作的不错，这样我就可以获取到全部的 issue 列表了。

然后后面其实还是差不多，至于评论分页又不一样了，既然 GitHub 只有分页 API，我也就半推半就啊不是将计就计吧。我才不要继续获取全部评论了，我也每次直接获取 GitHub 那边的一页就行了，每页个数则由主题作者传参进来。至于如何确定我要哪一页呢？和搜索功能一样，继续前端解析 query string 呗。根据 issue 评论总数计算一下有几页，然后生成几个链接放在页尾，每次加载时候解析一下参数确定当前页是哪个即可。当然，不要忘了 issue 顶楼不算评论，计算分页的时候也不要给它加进去！而且既然是这么分页，我也懒得把顶楼算在里面了（不然真的麻烦的要死啊后面分页和每页个数全乱了），所以假如你设置每页 10 个评论，那第一页其实有 11 个，别烦我，代码在那，不满意自己改……然后继续刀耕火种……

为了减轻负担，我没有实现太多的功能，比如时间戳我没有搞成什么几分钟几小时前，这种东西又不清晰又浪费带宽，我只搞了~~基于 marked 的 Markdown 渲染（必须的）和~~语法高亮，Markdown 渲染不是必须的，因为你可以 [设置 header 让 GitHub 直接返回 HTML](https://docs.github.com/en/rest/overview/media-types#comment-body-properties)。为了保证效果统一，这个 JS 只是建立了 HTML 布局，给每个元素添加了 class，具体的样式则完全是主题自己编写的，所以配合起来也比较好看。

最后的效果其实还可以，完整的脚本就是 [这个网站在用的 JS](/js/comment.js)，具体和主题的整合方法就慢慢翻 ARIA 的模板吧。唯一的缺点是 GitHub API 的频率限制太低，按照这个弱智的 issue 列表分页的话又不得不多一次读取仓库有多少 issue 的请求，假如你的 issue 太多估计也是问题？不过应该不会有那么多博文吧！我只有调试脚本的时候遇到过被 GitHub 提示等会的问题，所以对于访问者应该没什么影响。有影响以后再想解决办法（或者没办法）。

也许最好的办法是解决掉需求——要什么评论框？不就是破事水？如果有问题想联系作者电子邮件又不是不能用！（逃

# 其实你知道，烦恼（bug）会解决烦恼（bug）。

<div class="alert-red">更新（2022-03-21 16:46:46）：我终于发现 `fetch` 不会主动帮我做缓存的原因是我一直习惯性的在 DevTools 里面勾上 Disable Cache……所以下面这些其实都没有必要了，只要取消勾掉 Disable Cache，调试的时候就不会很快撞到查询频率次数限制了……</div>

<div class="alert-green">这一部分更新于 2020-08-17 10:34:00。</div>

GitHub API 推荐用户 [缓存之前的请求响应，然后根据缓存的响应的 Header 里面的 ETag 发送请求查询是否过期，若未过期则返回一个不消耗频率限制次数的 304 状态码](https://docs.github.com/en/rest/overview/resources-in-the-rest-api#conditional-requests)。我心想这也简单，那就在前端搞一个缓存就可以了。

然后我搜索了一番找到了 CacheStorage，看起来它是唯一一个跨标签页的基于 Session 的正宗的前端缓存。但是很显然 IE 又不支持，而且这个 API 基于 Promise 并且只能缓存 Response 对象，也就是说没办法简单的通过在 XHR 的时候判断一下跳过不支持的情况，要 Polyfill 则需要引入完整的 Promise 和 fetch/Response，所以我们做了一个~~艰难~~容易的决定——是时候去掉 IE 支持了！

于是我把请求 API 的函数改成了如下操作：

```JavaScript
let cachePromise = window.caches.open("cacheName");

// Fetching JSON with cache for GitHub API.
const cachedFetchJSON = (path, opts = {}) => {
  let cachedResponse = null;
  return cachePromise.then((cache) => {
    return cache.match(path);
  }).then((response) => {
    // No cache or no ETag, just re-fetch;
    if (response == null || !response.headers.has("ETag")) {
      return window.fetch(path, opts);
    }
    // Ask GitHub API whether cache is outdated.
    cachedResponse = response;
    opts["headers"] = opts["headers"] || {};
    opts["headers"]["If-None-Match"] = cachedResponse.headers.get("ETag");
    return window.fetch(path, opts);
  }).then((response) => {
    if (response.status === 200) {
      // No cache or cache outdated and succeed.
      // Update cache.
      cachePromise.then((cache) => {
        return cache.put(path, response);
      });
      // Cache needs an unconsumed response,
      // so we clone respone before consume it.
      return response.clone().json();
    } else if (response.status === 304 && cachedResponse != null) {
      // Not modified so use cache.
      return cachedResponse.clone().json();
    } else {
      // fetch does not reject on HTTP error, so we do this manually.
      throw new Error("Unexpected HTTP status code " + response.status);
    }
  });
};
```

当然理想很丰满现实很骨感，在不支持 CacheStorage 的浏览器里要 fallback 到不带缓存的版本，本来我以为很简单，但是……（下面开启吐槽时间。）

<blockquote class="center-quote">支持 IE 的前端的痛苦都是相似的，不支持 IE 的前端则各有各的痛苦。</blockquote>

为什么非 HTTPS + localhost 不能用 CacheStorage 啊，难道他们没考虑过在电脑上开发然后手机访问测试移动版吗？还是说他们打算在手机上起一个开发服务器？为什么 Firefox 在非 HTTPS 时限制 CacheStorage 的方法是在 Promise 里 reject 一个 Error 从而导致这个过程变成了异步的？为什么 CacheStorage 只能缓存 Response 而不是任意数据结构？Safari 不能完整支持 Response 对象也就算了，为什么移动版 Chrome 和 Firefox 也不支持？合着你们 fetch 返回的 Response 还不是 Response？这世界到底怎么了……

所以最后需要一个长长的基于 Promise 的判断加载函数：

```JavaScript
// 加载评论的时候才加载缓存。
let cachePromise = null;

let fetchJSON = uncachedFetchJSON;

const loadCache = (name) => {
  // Unlike in .then(),
  // we must explicit resolve and reject in a Promise's execuator.
  return new Promise((resolve, reject) => {
    if (cachePromise != null && fetchJSON !== uncachedFetchJSON) {
      return reject(new Error("Cache is already loaded!"));
    }
    // Old version browsers does not support Response.
    if (window.Response == null) {
      return reject(
        new Error("Old version browsers does not support Response.")
      );
    }
    const testResponse = new window.Response();
    // Safari and most mobile browsers do not support `Response.clone()`.
    if (testResponse.headers == null || testResponse.clone == null) {
      return reject(new Error(
        "Safari and most mobile browsers do not support `Response.clone()`."
      ));
    }
    // Chromium and Safari set `window.caches` to `undefined` if not HTTPS.
    if (window.caches == null) {
      return reject(new Error(
        "Chromium and Safari set `window.caches` to `undefined` if not HTTPS."
      ));
    }
    window.caches.open("CacheStorageTest").then((cache) => {
      fetchJSON = cachedFetchJSON;
      cachePromise = window.caches.open(name);
      return window.caches.delete("CacheStorageTest");
    }).then(() => {
      return resolve();
    }).catch((error) => {
      // Firefox throws `SecurityError` if not HTTPS.
      console.error(error);
      return reject(new Error("Firefox throws `SecurityError` if not HTTPS."));
    });
  }).catch((error) => {
    console.error(error);
  });
};
```

不管怎么样现在这个网站在支持 CacheStorage 和 Response 的浏览器上（似乎也就桌面版 Chrome/Firefox……）是缓存 GitHub API 的结果了，打开 DevTools 切到 Network 面板可以看到 GitHub API 返回的是 304 而不是 200，其他浏览器则 fallback 到无缓存的 fetch。当然其他浏览器不包含 IE 咯。

# 由俭入奢易，由奢入俭难。

<div class="alert-green">这一部分更新于 2020-08-18 12:25:00。</div>

我后来又仔细想了想，其实要兼容 IE 还是有办法的，首先 `fetch` 和 `Promise` 都有成熟的 polyfill，甚至 `URLSearchParams` 也有，只要写一段脚本在不支持的时候加载他们就可以了。然后去掉所有 IE 不支持的 ES6 特性，比如箭头函数、模板字符串、`for…of…` 循环以及 MapReduce（IE 竟然支持 `const` 和 `let` 真是惊到我了）。但是能做到并不意味着一定要做，人总是还要向前看的，现在是 2020 年，连罪魁祸首始作俑者微软都放弃了 IE，就算是照顾用户量，IE 用户也是可以忽略的那一部分了。既然我已经用 ES6 重写了，就不要想再让我为这种历史垃圾放弃我得到的好处了，从我开始写主题到现在丢掉 IE 支持也算是仁至义尽了，所以为什么不让这些用户支持一下 Firefox 呢？


