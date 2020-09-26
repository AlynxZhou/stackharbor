---
title: 固定 GNOME Shell 的输入法列表
layout: post
#comment: true
createdDate: 2020-09-26T10:40:00+0800
categories:
- 计算机
- GNOME
tags:
- GNOME
---
GNOME Shell 有个令人很不爽的“特性”，它的输入法列表使用的是最近使用优先排列。也就是说当你有三个或以上输入法的时候，比如我，我有英文简体中文和日语输入法，我经常在中英之间切换，这没什么，前两个总是中英所以按一次就可以在这两个之间切换，但假如我偶尔用了一次日语输入法，我的列表就被打乱了，我不清楚按几下才能切回中文，并且再切到英文也得看一眼才能知道。

<!--more-->

我不是很理解这个特性存在的意义，设置里面是可以手动调节输入法的顺序的，我明明调成了我想要的顺序，你就给我这个顺序好了，这样我闭着眼睛不用动脑子都能猜出来要按几下，比如从英文到日语按两下，中文到日语按一下等等。可能有些人的脑子长得比较擅长模拟最近使用优先排列？反正我不行。

既然感觉不爽那就动手处理一下好了，最近看了一些有关写 GNOME Shell 扩展的文档，所以写个扩展解决一下就可以了。为什么不直接提交给上游？因为上游一开始是固定顺序的，但是很久以前某个人加了这个“特性”，现在如果提个请求说删掉这个特性，势必会陷入一场“用户到底是喜欢最近使用优先排列还是固定排列”的争论，这肯定很难得出结论（毕竟大部分的人实际上是不需要使用输入法的英语用户以及只有两种输入法的用户！），并且按照 GNOME 上游的习惯他们也是不愿意为了这个多添加一个开关的。所以比起在拉锯战上浪费时间，先搞一个能用的才是我的风格。至于升级之后扩展挂掉……不就是在上游里和其他代码一起被重构和我自己单独重构的区别吗？只要我还在用应该就会持续更新了。

具体的解决方法比较 dirty，是我从别的扩展里学来的：把 GNOME Shell 里面的类的原型上的方法替换成自己的，就可以修改实例调用时的函数了（也算 JS 特性之一），不过不要用箭头函数，因为显然我们希望 `this` 是调用时的上下文也就是实例，而不是绑定到当前上下文。

因为这算是我第一个扩展所以也多少记录一下踩的坑。

首先 Gjs 的导入和 Node.js 的导入是不一样的，它通过一个 `imports` 对象引入其他库，比如通过 GI 导入的就在 `gi` 下面，因为是 GNOME Shell 扩展所以可以访问 GNOME Shell 的 JS 库，就是简单地把 JS 路径换成对象的 key 然后 JS 文件里所有的 `var` 和 `function` 都会被导出。比如导入 `Main` 就是 `imports.ui.main.Main`。

然后就是怎么知道要修改什么以及如何获取到相关对象，不过因为 GNOME Shell JS 部分经常重构，也没什么完整的文档，反正只能多花时间看代码吧，而且它的结构其实比看起来的要复杂，所以经常需要仔细翻来翻去的。比如 GNOME Shell 的输入法部分很多人认为是需要修改 iBus，实际上 GNOME Shell 只是调用 iBus 作为后端，自己处理状态和界面，这部分的代码都在 `js/ui/status/keyboard.js` 里面。

扩展主要有 `init()`，`enable()` 和 `disable()` 三个函数，`init()` 在 GNOME Shell 加载扩展时候调用，我这个显然不需要。`enable()` 是你在 Extensions app 里面打开开关时候调用的，`disable()` 是关掉开关时候调用的。

在 `enable()` 里面有几个需要我修改的地方，一个是阻止 `InputSourceManager` 在输入法切换之后的最近使用优先排列，解决方法很简单，需要自己替换掉 `_currentInputSourceChangedOrig` 函数，注释掉 <https://gitlab.gnome.org/GNOME/gnome-shell/-/blob/gnome-3-36/js/ui/status/keyboard.js#L447-453> 这一段更新代码。

当然光有这个还是不行的，这样假如你是先切换过再打开扩展，实际上列表是你开启扩展之前的状态而不是用户设置的顺序，所以我们还需要在打开扩展之后更新它的列表，让它直接读取用户设置。更新列表的函数是 `_updateMruSources`，假如检测到当前列表为空，会先从一个缓存的 gsettings 里读取之前存储的最近使用优先排列列表，这显然是很恶心的所以要注释掉 <https://gitlab.gnome.org/GNOME/gnome-shell/-/blob/gnome-3-36/js/ui/status/keyboard.js#L504-522> 这一段。之后它会先加载当前列表里的，然后再把用户列表里增加的当前列表里没有的加到后面，因为我们已经决定要清空当前列表并且不加载 gsettings 里面的缓存，所以这个当前列表肯定是空，那直接加用户列表就行了，所以注释掉 <https://gitlab.gnome.org/GNOME/gnome-shell/-/blob/gnome-3-36/js/ui/status/keyboard.js#L525-533>。这样我们后续只要清空 `_mruSources` 设置 `_mruSourcesBackup` 为 `null` 然后调用 `_updateMruSources` 就可以了。

然后我们需要获取运行时的这个 `InputSourceManager` 实例，这个实例没有被绑定到 `Main` 对象上，不过我阅读代码发现它是个单例模式，就是说在 `js/ui/status/keyboard.js` 有一个 `_inputSourceManager` 变量，然后有个 `getInputSourceManager()` 的函数，被调用时候如果有就返回 `_inputSourceManager` 否则创建一个赋值返回，其他代码都用的这个，所以我们也导入这个就行了。

然后你会发现另一个弱智的地方，怎么每次按下切换键，切换框都是从第一个切换到第二个？不是应该从我当前的切换到下一个吗？这个对于当前输入法总在第一个的最近使用优先排列是可以的，但在我们这个场景选中的并不总是第一个，所以需要修改。这部分函数是 `_switchInputSource`，可以看到它只是展示了一个 `InputSourcePopup`，而 `InputSourcePopup` 继承的是 `imports.ui.switcherPopup.SwitcherPopup`，这个类有一个叫做 `_selectedIndex` 的变量用于选择下一个上一个时候的计算，而且它默认是 0！不能通过参数初始化！真是头秃，不过我们可以在创建完切换框但展示之前单独设置这个值就行了，所以我在 <https://gitlab.gnome.org/GNOME/gnome-shell/-/blob/gnome-3-36/js/ui/status/keyboard.js#L412> 的下一行插入如下代码：

```JavaScript
if (this._currentSource != null) {
  popup._selectedIndex = this._mruSources.indexOf(this._currentSource);
}
```

因为我们不一定总有 `_currentSource` 所以还是要检查一下，如果没有的话让它从 0 开始也无所谓。

然后还有一个比较头痛的是快捷键是绑定的回调函数，回调函数又绑定了 `this`，所以我们光修改原型是改不了被回调的函数的，这个也简单，我们需要读一下 `InputSourceManager` 的 `constructor` 的代码，然后删掉它在 `Main.wm` 里面绑定的组合键重新绑定成我们的，就是这样：

```JavaScript
Main.wm.removeKeybinding("switch-input-source");
_inputSourceManager._keybindingAction =
  Main.wm.addKeybinding(
    "switch-input-source",
    new Gio.Settings({"schema_id": "org.gnome.desktop.wm.keybindings"}),
    Meta.KeyBindingFlags.NONE,
    Shell.ActionMode.ALL,
    InputSourceManager.prototype._switchInputSource.bind(_inputSourceManager)
  );
Main.wm.removeKeybinding("switch-input-source-backward");
_inputSourceManager._keybindingActionBackward =
  Main.wm.addKeybinding(
    "switch-input-source-backward",
    new Gio.Settings({"schema_id": "org.gnome.desktop.wm.keybindings"}),
    Meta.KeyBindingFlags.IS_REVERSED,
    Shell.ActionMode.ALL,
    InputSourceManager.prototype._switchInputSource.bind(_inputSourceManager)
  );
```

同样我们也不要忘记绑定 `this`，实际上我们希望调用的时候绑定的 `this` 是那个单例，那直接 `bind` 它就好了。

但是你会发现这个弱智的家伙没有按我们想象的工作！这是什么意思！仔细阅读代码之后我发现有如下逻辑：你按下的第一次组合键其实并不是算在那个弹框的按键回调里面，而是我们通过构造函数传递进去的，然后它分析这个传进去的按键是哪一种，调用 `_initialSelection` 执行第一次切换，而这个家伙更弱智了！明明有 `_selectedIndex` 它不用，竟然用硬编码的倒数第一个和第一个！真有你的啊！我不太敢修改 `SwitcherPopup` 因为还有别的东西使用它，那就修改 `InputSourcePopup` 这个子类吧，其实就是把 `InputSourcePopup.prototype._initialSelection` 这个函数原来的的 `this._select(this._items.length - 1);` 换成 `this._select(this._previous());`，`this._select(1)` 换成 `this._select(this._next())`（1 其实是 0 + 1 的意思），不但功能增加了，可读性也提升了！

现在搭配起来应该和我们的需求一致了！但假如我关掉扩展之后希望列表是打开之前的状态怎么办！还记得之前说的那个 `_updateMruSources` 会读取 gsettings 吗？这个 gsettings 实际上在每次切换输入法的时候都会写入当前状态，那我们只要让它开启扩展时候不要写入，关掉扩展恢复的时候再更新不就读取了之前的状态吗。所以需要修改 `InputSourceManager.prototype._updateMruSettings`，注释掉 <https://gitlab.gnome.org/GNOME/gnome-shell/-/blob/gnome-3-36/js/ui/status/keyboard.js#L432-438>。

总结一下其实就是在 `enable` 的时候修改这些函数，然后获取单例，重新绑定快捷键，然后清空当前的列表重新更新列表，然后为了避免 bug，我们总是激活列表里的第一个输入法：

```JavaScript
if (_inputSourceManager._mruSources.length > 0) {
  _inputSourceManager._mruSources[0].activate(true);
}
```

`disable` 的时候同样是把函数修改回去，然后获取单例，重新绑定快捷键这样它又绑成了原来的函数，然后清空当前的列表重新更新列表这样它就恢复到开启之前的顺序了，接下来同样地，因为最近使用优先列表的第一个元素肯定是正在用的，所以我们也还是激活第一个输入法就可以了。

完整的项目参见 [GitHub](https://github.com/AlynxZhou/gnome-shell-extension-fixed-ime-list/)，Arch Linux 用户也可以从 AUR 或者 Arch Linux CN 源安装 `gnome-shell-extension-fixed-ime-list`。

*Alynx Zhou*

**A Coder & Dreamer**
