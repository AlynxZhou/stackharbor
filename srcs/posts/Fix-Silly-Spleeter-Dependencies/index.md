---
title: 解决 Spleeter 愚蠢的依赖问题
layout: post
#comment: true
created: 2020-11-26T16:43:12
categories:
- 计算机
- 编程
- Python
tags:
- Python
- Spleeter
---
我倒不是对机器学习有什么莫名的偏见。事实上有些只能用机器学习搞定的东西我也很支持用机器学习解决，比如 waifu2x 这种增加图片分辨率的或者 Spleeter 这种分离人声和伴奏的，用传统的分离人声的方法就是不能完美解决这类问题，而机器学习模糊分类则可以无限接近完美解决。我讨厌的有两个，一个是传统方法很好解决的东西非要用机器学习解决，另一个是混乱的机器学习项目，后者更严重。

<!--more-->

不知道是因为什么原因，许多机器学习从业者似乎都缺乏整理代码和依赖的能力——能轻松地把一个这类开源项目打包简直是奇迹，更多时候别提打包了，你想自己安装然后跑起来都不太现实，哪怕是同样做机器学习的其他人也有这样的苦恼：实现者似乎把论文写好然后在自己电脑上能跑起来就心满意足了，丢出来一份没什么文档的代码，不折腾几天根本不知道这坨代码需要装些什么才能跑起来。对我这种洁癖用户就更严重了，我可不希望 pip 在我的系统里塞一堆乱七八糟的东西，这个需要其他项目才能避免全局安装的程序比起 npm 真是差远了，解决依赖的能力也远不如 pacman 和 npm。

然后有人发明了 conda 和 docker 这样的 ~~辣鸡~~ 项目来拯救其他的辣鸡，但事实上不过是变得更糟而已——它们给了一些懒人名正言顺不维护项目的借口——反正我丢一个 conda 配置或者 docker 文件上去，你们拉上一大堆和我一样的过期依赖就能跑了，至于占了你多少硬盘，干不干净，关我什么事？如果不是物理限制，恐怕这些懒人会把整个宇宙都塞进去。而且他们再也不会管版本更新，什么 tensorflow 2 与我何干？用了我的 docker 你的系统里就会有几百个不同的过期的 tensorflow 1，买硬盘就完了呗？这和买显卡就完了呗还真是一路货色，这样依赖商业公司的产品，总有一天感觉会被割韭菜。

今天我用到的这个 Spleeter 某种意义上也是这样的一坨辣鸡，它的仓库里面的 README 是完全过时的，但这起码是件好事：代码还在更新。只是你按照文档是没办法顺利的搞定它的，我总结了一下我的解决方案，可能不适合其他人，但我大概知道怎么解决了。

这个文档有多离谱呢？它说你如果使用 GPU 加速的版本，必须从 conda-forge 安装，pip 和 GitHub 的版本都只有 CPU——那你这也是按揭开源？然后整个文档没有一个地方告诉我我需要单独下载训练好的模型才能跑起来，直接运行程序则丢一个段错误——可能他们搞机器学习的人觉得下模型是常识不需要说吧。

如果直接按它的方法运行 `conda install -c conda-forge spleeter-gpu`，你会得到一个不能用 GPU 的 GPU 版，这实在是太搞笑了。我捏着鼻子用 conda 就是为了能让你一步把这一坨东西给我弄好，现在你说你弄不好？

但是有一点好处就是实际上他们的代码已经更新到支持 tensorflow 2 了，所以其实完全不需要用那个过期的 conda-forge 的版本，你完全可以直接在 Python 3.8 里面 `pip install spleeter-gpu` 安装最新的版本——然后我又遇到了依赖问题，我的一些系统软件包依赖 numpy，然后 Arch 官方源里的 numpy 版本还是比 spleeter-gpu 依赖的 tensorflow 依赖的 numpy 版本新——你们写 Python 的人真麻烦，版本号兼容性是可以随便 break 的吗？

所以这时候我还是用 conda 解决，总之就是搞一个和系统独立的虚拟环境（npm：这不应该是内置功能吗？还需要用商业软件？）。总之[去 TUNA 搞一个 Miniconda 3 最新的安装包来](https://mirrors.tuna.tsinghua.edu.cn/help/anaconda/)，然后直接安装，看着它往你的 shell 配置里塞一坨辣鸡（我把实际用到的命令拿出来做了个 alias，这样就可以只在我需要的时候打开 conda 了）。

为什么使用 miniconda 而不是 anaconda？因为 anaconda 带的那个图形界面根本用不了，miniconda 够用了。

然后创建一个 Python 3.8 的环境，高了低了都不行，真是难伺候，同时别忘了带上 cuda 和 cudnn，不然它一声不吭的就会只能用 CPU：

```
$ conda create -n spleeter python=3.8 cudatoolkit cudnn
```

cuda 不叫 cuda，叫 cudatoolkit 就离谱。

然后切进去：

```
$ conda activate spleeter
```

然后装 `spleeter-gpu` 到 conda 新创建的这个 Python 环境：

```
$ pip install spleeter-gpu
```

所有的依赖应该 pip 都会解决，但这个弱智有时候还会说我本机已经装了 numpy 1.19.4，比 tensorflow 需要的版本高，但我都创建虚拟环境了你还读取我系统的干嘛？不过其实好像也不影响使用，或者此时可以 `pip install numpy==1.18.5`。

然后到 <https://github.com/deezer/spleeter/releases> 去下载训练好的模型，这也是个弱智的地方：哪有 tar 打包不把目录本身打进去的？然后还得给这个程序创建一个工作目录，因为它是写死的到当前目录下面的 `pretrained_models` 下面去找模型。

```
$ mkdir -p spleeter/pretrained_models
$ cd spleeter/pretrained_models
```

我这里用 2stems 的模型示范，因为我只需要分离人声和伴奏：

```
$ wget -c 'https://github.com/deezer/spleeter/releases/download/v1.4.0/2stems.tar.gz'
$ mkdir 2stems
$ cd 2stems
$ tar -xpvzf ../2stems.tar.gz
```

因为他们打 tar 包时候没把目录打进去，所以别忘了自己创建目录！

然后回到你的工作目录就可以用了，我这里结构是 `spleeter/pretrained_models/2stems` 所以工作目录就是 `spleeter`：

```
$ cd ../../
$ spleeter separate -i 你要处理的歌曲 -p spleeter:2stems -o 输出目录 -B tensorflow
```

如果你想使用其他的几个模型，那就把 2stems 改成其他模型的名字，但是这里还有个开发者脑子抽了的地方：带有 `-finetune` 的是高品质模型，它们的模型目录名字应该是比如 `2stems-finetune`，但是参数名字却不是这个而是 `-p spleeter:2stems-16kHz`，文档里当然是没找到的，我觉得应该揍开发者一顿让他老实写文档（不过我又看了一下，这个 finetune 对于分离音轨没什么用好像）。

我这里必须使用 `-B tensorflow` 才会走显卡加速。

用完了就可以 `conda deactivate` 退出虚拟环境，要用的时候别忘了 `conda activate spleeter` 切换进来，

遇到类似问题的同学可以参考我的文章，但是因为这个处理过程影响因素太多了，如果你的不能用我也没什么办法。

回头一看，这个项目犯了一大堆禁忌：难以解决的依赖，写死的模型路径还有匮乏的文档，导致配环境就要配一大堆。当然，好在他们还是在努力更新跟上依赖而不是撒手不管让它慢慢死去，并且功能非常好。真正对开发者友好应该是不需要配环境的，比如在 Linux 下面开发软件，包管理已经帮你考虑好各种依赖了。只有像 Windows 或者 Android 这种不以开发者为中心或者是许多机器学习项目这种“数据好看就行”的地方才会有这么多麻烦。真的很希望这些人能补一点务实的基础，不要让他们的软件这么难用。

*Alynx Zhou*

**A Coder & Dreamer**
