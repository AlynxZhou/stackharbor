---
title: "Node 的 http.request() 需要对 response 进行错误处理"
layout: post
#comment: true
created: 2024-01-03T15:21:05
#updated: 
categories:
  - 计算机
  - 编程
  - JavaScript
tags:
  - JavaScript
---
我发现有些时候 Telegram bot 很适合用来 host 一些我自己要用的服务，因为只要通过手机上的聊天框就可以控制了，不需要我自己写一些什么后台页面。为了让构建和安装一个新 bot 的过程尽量简单，我自己用 Node 写了一个 [没有外部依赖的 Telegram bot 框架](https://github.com/AlynxZhou/azbot-telegram/)。完全使用 Node 自带的模块比较麻烦的一点就是你需要自己基于 `http.request()` 进行封装，因为原版基于 `EventEmitter` 的接口写起来实在是太复杂了。

<!--more-->

把 `http.request()` 封装成 Promise 比一般的 API 要难一点，但也不是完全做不到，比如 [官方文档上的示例代码](https://nodejs.org/api/http.html#httprequestoptions-callback) 是这样写的（复制这么长一段不是我要占字数而是我真的被它坑了）：

```JavaScript
import http from 'node:http';
import { Buffer } from 'node:buffer';

const postData = JSON.stringify({
  'msg': 'Hello World!',
});

const options = {
  hostname: 'www.google.com',
  port: 80,
  path: '/upload',
  method: 'POST',
  headers: {
    'Content-Type': 'application/json',
    'Content-Length': Buffer.byteLength(postData),
  },
};

const req = http.request(options, (res) => {
  console.log(`STATUS: ${res.statusCode}`);
  console.log(`HEADERS: ${JSON.stringify(res.headers)}`);
  res.setEncoding('utf8');
  res.on('data', (chunk) => {
    console.log(`BODY: ${chunk}`);
  });
  res.on('end', () => {
    console.log('No more data in response.');
  });
});

req.on('error', (e) => {
  console.error(`problem with request: ${e.message}`);
});

// Write data to request body
req.write(postData);
req.end();
```

那对于一个 POST 请求，我就可以这样封装：

```JavaScript
import * as http from "node:http";
import {Buffer} from "node:buffer";
const post = (url, body, headers = {}) => {
  const opts = {
    "method": "POST",
    "timeout": 1500,
    "headers": {}
  };
  for (const [k, v] of Object.entries(headers)) {
    opts["headers"][k.toLowerCase()] = v;
  }
  if (!(isBuffer(body) || isString(body))) {
    body = JSON.stringify(body);
    opts["headers"]["content-type"] = "application/json";
    opts["headers"]["content-length"] = `${Buffer.byteLength(body)}`;
  }
  return new Promise((resolve, reject) => {
    const req = http.request(url, opts, (res) => {
      const chunks = [];
      res.on("data", (chunk) => {
	chunks.push(chunk);
      });
      res.on("end", () => {
	resolve(Buffer.concat(chunks));
      });
    });
    req.on("error", reject);
    req.write(body);
    req.end();
  });
};
```

反正流程无非是创建 request，然后在 response 里面收集 data 到 buffer，然后处理 request 的 error，再把 body 写到 request 里面。看起来很简单毕竟官方文档也这么写的对吧！然后就掉进坑里了。

我的 Telegram bot 设置是要不停通过 HTTP 轮询获取更新，为了保证能一直轮询下去，就要在遇到错误的时候 catch 住简单处理，然后继续进行下次轮询。但明明我已经在可能出现错误的时候都处理了，bot 还是会在跑了几天以后遇到错误（通常是 `read ETIMEOUT`）然后完全停住，只能手动重启。我对此绞尽脑汁，但是想不出哪里有问题，同时因为这个要 bot 跑一段时间才能复现，也很难 debug，我甚至手动打了 log 来看是轮询停住了还是轮询没有停但却一直得到空的结果，实际证明是遇到错误停住了，但我不是已经进行错误处理了吗？

这个问题实在是找不到什么参考，我尝试了一些没有意义的办法，最后差点去翻什么 axios 之类的代码看他们如何解决的了。不过我在此之前想了一下，会不会是因为不仅要写 `req.on("error", reject);`，还要写 `res.on("error", reject);` 来处理 response 的错误，否则 Node 就会直接把相关的错误抛出来停掉？其实我心里觉得不太可能，毕竟 **示例代码里根本都没有写这句**，但我还是本着没办法的办法写上去了：

```patch
diff --git a/azbot-telegram/bot-utils.js b/azbot-telegram/bot-utils.js
index 42e002e..f90a8eb 100644
--- a/azbot-telegram/bot-utils.js
+++ b/azbot-telegram/bot-utils.js
@@ -360,13 +360,13 @@ const post = (url, body, headers = {}) => {
   return new Promise((resolve, reject) => {
     const req = https.request(url, opts, (res) => {
       const chunks = [];
+      res.on("error", reject);
       res.on("data", (chunk) => {
         chunks.push(chunk);
       });
       res.on("end", () => {
         resolve(Buffer.concat(chunks));
       });
     });
     req.on("error", reject);
     req.write(body);
```

然后问题就神奇的解！决！了！我的 bot 连续跑了十天半个月也没有挂，我心里这个气啊，为什么官方文档里的示例一点没提到要对 response 的 error 事件进行处理呢？甚至在网上也很难找到相关的信息，我推测是大部分人并不从头自己封装 HTTP 模块而是直接使用现成的库比如 axios，然后可能有人发现过这问题就简单地给 axios 提了这么一个 fix，就再也没人提起过这件事了。总之还是希望官方文档能更新一下示例代码吧。
