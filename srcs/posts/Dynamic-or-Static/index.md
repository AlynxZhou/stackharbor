---
title: 动态？静态？
layout: post
comment: true
created: 2016-06-12 16:55:14
categories:
  - 网站
tags:
  - 网站
---
本来hexo静态博客用的好好的，结果在静好的鼓动下又想试试动态站，选择困难又犯了……

<!--more-->

静态站相比动态站的优点就是资源消耗小（就是几个html也不用cpu运算啊）所以可以挂到pages上去，省了vps的开销，并且文件都是Markdown的文本文件，自从用了Gentoo我就更喜欢文本文件的管理方式了，对数据库没什么好感，在本地用Emacs写也很棒。而且我不会用PHP，所以还不如静态站简单粗暴来得好一点。

但是静态站是不能交互的，这导致的最大的缺点就是没有自己的评论系统一类的。虽然Disqus做的很好但是国情需要啊喂，还有就是现在它默认变成了引导用户注册！考虑了一下受众我还是放弃了……多说比较接地气，但是它的服务质量始终不能让人放心……GitHub上有一个comment(it)，把评论直接作为commit推上去，不过目前仅支持jekyll……其他的动态服务比如不蒜子数据统计还是很可靠，搜索页有现成的Local Search让它变成静态的，所以最不靠谱的还是多说？

如果换成动态站的话首先是模板的问题，hexo的NexT主题的动画实在是很精致，但整个主题有点太简洁了，其实并不符合我的预期。虽然typecho有一个足以以假乱真的移植版，但是还是有些细节不一样，一旦找到合适的新主题也许我会切换过去，但是这样就要放弃Markdown文本文件……同样的还要租vps……我是个穷人……

但显然动态站可以更好地支持自建评论系统，不过为了这么一个特性见过去是否有点得不偿失？

目前为止hexo都可以很好的满足需求，如果有了新的需求我想我会换成动态站？不过最好的办法也许是自己造轮子？把静态部分和动态部分分开？这样就可以保持hexo目前的特性？


