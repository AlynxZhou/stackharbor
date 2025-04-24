---
title: YubiKey 和 GNOME 和智能卡登录
layout: post
#comment: true
created: 2022-11-28T18:24:04
updated: 2025-04-24T17:42:09
categories:
  - 计算机
  - GNOME
tags:
  - 计算机
  - GNOME
  - YubiKey
---
最近我终于决定买了一个 YubiKey 5C，说出来不怕各位笑话，我买这玩意最初的动机只是觉得每次开机和解锁输密码太麻烦（但是为什么我不觉得 `sudo` 输密码麻烦呢？）。这还和我之前处理了一个 openSUSE 的 PAM 问题有关，我发现 GDM 有好几种不同的 PAM 配置，除了平时用的 `gdm-password` 密码登录，还有 `gdm-fingerprint` 指纹登录和 `gdm-smartcard` 智能卡登录。我一开始是打算买个指纹传感器的，查了一下 fprintd 的文档，支持的型号并不多，而且在淘宝上问客服 USB Product ID 和 Vendor ID 显然得不到回答，就退而求其次买智能卡了，而搜索智能卡得到最多的结果就是 YubiKey，进入一个不了解的领域之前和大部分人选一样的一般不会错太多，于是就下手了。

话说回来智能卡登录，如果你搜索 YubiKey 相关的文章，绝大多数都会告诉你把 `pam_u2f.so` 加到需要密码登录的 PAM 配置里，比如 `sudo` 或者 `gdm-password`，但这显然不是我想要的，我要的方案不是替换密码登录，而是和密码登录平行的配置文件，我知道 GNOME 有已经写好的智能卡配置，但是任何地方都搜不到如何启用，设置里没有相关选项，连 Arch Wiki 给的方案都只是用 `pam_u2f.so`。Red Hat 的支持文档里倒是提到了智能卡登录，然而用的却是他们自己的某个工具配置的。显然这是个起夜级 feature，最好的办法也许是找个起夜级 Linux 的桌面工程师来问问，哦什么我自己就是起夜级 Linux 桌面工程师，那没事了。还要说的一件事是怎么想智能卡这东西都和安全相关，而我自己不是专业的安全行业人士，所以我不会尝试解释清楚和安全相关的一些名词，以及如果我哪里真的写错了，希望专业人士多多指点，我肯定改。

总之相信你自己因为你才是职业选手，我还是自己看看这东西怎么弄吧，毕竟用 Arch 再用 GNOME 同时还打算搞 GNOME 的智能卡登录的人没几个，所以 Wiki 没有倒也正常。首先肯定是看 `/etc/pam.d/gdm-smartcard` 这个文件，里面别的看起来都比较正常，只有一行看起来和智能卡有关系：

```
auth       required                    pam_pkcs11.so        wait_for_card card_only
```

线索是有了，看来我需要这个 `pam_pkcs11.so`，虽然我也不知道这是什么，先搜一下哪个包有这个文件比较好。`pacman -F pam_pkcs11.so` 竟然没有返回任何结果，我确定不是我的 pacman 数据库没更新，那只能去浏览器里搜索了，最后我搜到了 <https://github.com/OpenSC/pam_pkcs11>，虽然我也不知道 `PKCS#11` 是个什么玩意，但反正它是个 PAM 模块，既然不在官方仓库里，那大概率 AUR 里有人打包了，于是直接 `paru pam_pksc11` 就装了一个上来。

但装是装好了，也看不出来这玩意和 YubiKey 有什么联系，我大概是搜索了 PKCS YubiKey 然后搜到了 YubiKey 给的文档 [Using PIV for SSH through PKCS #11](https://developers.yubico.com/PIV/Guides/SSH_with_PIV_and_PKCS11.html)，好吧虽然我不是要用来 SSH 但是多半也有点用。看下来反正这个东西和 YubiKey 的 PIV 功能有关，我把 PIV 相关的文档都看了一遍，结果是云里雾里，相当没有头绪。一大堆文档告诉你各种各样的需求要做什么，但是几乎没怎么说这都是什么，于是恰好我的需求不在列表里我就不知道怎么办了。我又回头去看 `pam_pkcs11` 的文档，它写了一长串的东西，我反复看了几遍之后发现只要看 [第 11 节的 HOWTO 部分](http://opensc.github.io/pam_pkcs11/doc/pam_pkcs11.html#HOWTO) 就可以了。虽然我也不太清楚它都在说什么，但是至少这里告诉我说需要一个 root CA certificate，但我是个人使用哪来的这玩意，再回头看 YubiKey 的那篇文档里面恰好提到了什么 self-signed certificate，我拿这个试一试，结果成功了。为了方便参考，下面我就不讲我是怎么倒推这些奇怪的需求的了，而是顺序讲一下都需要配置什么。

首先如果你像我一样刚买了一个 YubiKey 打算利用它的 PIV 功能，那你得先初始化它，也就是改掉默认的 PIN，PUK 和管理密钥，这个可以通过官方的 YubiKey Manager 软件来操作：

```shell
# pacman -S yubikey-manager
```

之前这个软件还有个 Qt 写的 GUI 版本，但是已经被官方放弃了，不过本来 GUI 也不是很好用，所以建议还是用命令行：

```shell
% ykman piv access change-pin
% ykman piv access change-puk
% ykman piv access change-management-key --generate --protect --touch
```

默认 PIN 是 `123456`，默认 PUK 是 `12345678`，而管理密钥是个特别长的一串，用 `--generate` 可以让 `ykman` 给你生成一个，`--protect` 则是把这个直接存到 YubiKey 里面并用 PIN 保护，`--touch` 则是说每次要管理密钥的时候需要你摸一下。我也不是很懂，也许写进去以后需要的时候就不用自己背这玩意而是输 PIN 就行了吧，反正建议看官方文档 [Device setup](https://developers.yubico.com/PIV/Guides/Device_setup.html) 和 `ykman` 的 `--help`。

我的建议是不要看太多官方文档，因为它一会告诉你用 `yubico-piv-tool` 创建密钥，一会告诉你说可以用 `openssl` 创建密钥，一会又告诉你可以用 `pkcs11-tool` 搭配 `libykcs11.so` 创建密钥，算了吧，头都看晕了，我的测试是用 `yubico-piv-tool` 就可以了。

```shell
% paru yubico-piv-tool
```

在 `9a` 这个槽创建一个 key 并把它的公钥写出来，为什么是 `9a` 好像因为这是第一个槽来着，自己去查官方文档吧，也可以写到别的槽里面：

```shell
% yubico-piv-tool -s 9a -a verify-pin -a generate -o public.pem
```

需要先输入 PIN，然后灯闪的时候需要摸一下 YubiKey，它就开始生成了。

还要给这个密钥生成一个签名：

```shell
% yubico-piv-tool -s 9a -a verify-pin -a selfsign-certificate -S "/CN=Alynx Zhou/" -i public.pem -o cert.pem
```

注意 `CN=` 后面的部分，这里会被 `pam_pkcs11.so` 用来验证这个智能卡属于系统里面哪个用户，所以简单的话直接写你的登录用户名，当然你像我一样不想写用户名也是有办法对应的，同样要输入 PIN。

再把证书导回到同一个槽，我也不知道为什么，文档说了我照做了（大概是为了方便携带，需要证书的时候可以直接从 YubiKey 里面导出）：

```shell
% yubico-piv-tool -s 9a -a verify-pin -a import-certificate -i cert.pem
```

还是要输入 PIN 然后灯闪的时候摸一下。

-------

更新（2025-04-24）：我发现生成的证书就像 HTTPS 的证书一样其实是会过期的，默认的有效期是一年，到期了需要重新生成一个证书。我大概理解了这玩意是怎么回事并且搞清楚如何用 `ykman` 配置了所以我这里再记录一下怎么用 `ykman` 更新证书。

整个的流程其实是你有一对私钥公钥用来签发证书，私钥在你的 YubiKey 里，公钥则是一个文件，使用公钥签发证书，然后把证书丢给 pam_pkcs11，pam_pkcs11 请求卡片用私钥验证证书是否符合。为了方便，证书可以导入 YubiKey，也可以从 YubiKey 里导出。

首先可以用 `ykman piv info` 查看一下目前证书的信息。

如果已经有一个过期的就用 `ykman piv certificates delete 9a` 删掉旧的证书。

然后用 `ykman piv certificates generate -s "CN=Alynx Zhou" 9a public.pem` 签发一个新证书，注意这里字符串的格式和 `yubico-piv-tool` 不一样，这里用逗号而不是斜杠做分隔符。

和 `yubico-piv-tool` 不一样，此时证书是直接生成在卡片里的，要交给 pam_pkcs11 的话得用 `ykman piv certificates export 9a cert.pem` 导出成文件。

-------

到这里 YubiKey 的配置就结束了。

要在系统上使用智能卡验证需要安装系统上和智能卡交互的软件包：

```shell
# pacman -S ccid opensc pcsclite
% paru pam_pkcs11
```

启动一个相关的 daemon，或者启动 socket 也行，需要的时候它就自己起来了：

```shell
# systemctl enable --now pcscd.socket
```

如果我没漏掉什么乱七八糟的，就可以配置 PAM 模块了，它有一个配置目录叫 `/etc/pam_pksc11`，首先你要把上面生成的证书放到 `/etc/pam_pkcs11/cacerts`。

```shell
# cd /etc/pam_pkcs11/cacerts
# cp PATH_TO_YOUT_CERT/cert.pem ./
```

你要在同一个目录下面运行一个什么什么 hash 命令生成一个 hash：

```shell
# pkcs11_make_hash_link
```

接下来你要去搞它的配置文件，先复制一个样本过来：

```shell
# cp /usr/share/doc/pam_pkcs11/pam_pkcs11.conf.example /etc/pam_pkcs11/pam_pkcs11.conf
```

好像其实也没什么需要改的。文档说默认的配置用的是 OpenSC 的 PKCS#11 库，虽然 YubiKey 的文档一直跟你说什么 `libykcs11.so`，我的测试结果是不用理它，通用的接口就够了，以及这个 `libykcs11.so` 是属于 `yubico-piv-tool` 这个包的。

假如你刚才 `CN=` 后面写的不是你的用户名，那你需要一些配置告诉 `pam_pkcs11.so` 你这个证书对应的哪个用户，这一步在它的配置文件里叫 `mapper`。默认启用了一些 mapper 比如 `pwent`，这个就是把 `CN=` 后面的内容和 `/etc/passwd` 里面的用户名做匹配，但是如果你像我一样写的是全名，那就需要另一个默认启用的模块叫 `subject`。至于 subject 是什么需要运行下面这个命令：

```shell
% pkcs11_inspect
```

它会输出各种 mapper 对应的 data，比如 `pwent` 输出的就是 `Alynx Zhou`，`subject` 输出的则是 `/CN=Alynx Zhou`。我们需要复制一个 `subject_mapping` 配置文件的样本过来：

```shell
# cp /usr/share/doc/pam_pkcs11/subject_mapping.example /etc/pam_pkcs11/subject_mapping
```

在这个文件后面加一行：

```plain
/CN=Alynx Zhou -> alynx
```

我的用户名是 `alynx`，你可以换成你自己的。

到这一步 `pam_pkcs11.so` 这个模块已经可以通过智能卡验证你的身份了。

-------

<div class="alert-red">更新（2025-04-24）：直接使用 p11-kit 的 MR 已经合并了，所以下面讲配置 NSS 数据库的部分都不需要做了。</div>

但是如果你火急火燎兴高采烈的重启了系统，GDM 还是会和你要密码。原因其实很简单，虽然现在 `/etc/pam.d/gdm-smartcard` 已经可用了，但 GDM 只有在检测到智能卡之后才会调用这个文件尝试智能卡登录，很显然它没检测到智能卡。

这里就比较难搞清楚了，我智能卡插的好好的，上面各种程序都能用，为什么你检测不到？我尝试用什么 GDM YubiKey 之类的关键词搜索了半天，也没人告诉我 GDM 到底怎么检测智能卡的。没有办法还是读代码吧，GNOME Shell `js/gdm/util.js` 里面的逻辑是通过 D-Bus 的 `org.gnome.SettingsDaemon.Smartcard` 获取智能卡信息，那我打开 D-Feet 从 Session Bus 里面找到这个，直接运行 `org.gnome.SettingsDaemon.Smartcard.Manager` 的 `GetInsertedTokens`，什么都没有。

根据 D-Bus 的信息，很显然这个接口是 `gnome-settings-daemon` 的 `smartcard` 插件提供的，我大概是搜索了什么 gsd-smartcard PKCS#11 的关键字之后找到了 <https://gitlab.gnome.org/GNOME/gnome-settings-daemon/-/merge_requests/208>，其实我一开始也没太看懂这是什么意思，但得到一些有用的信息：

- `gsd-smartcard` 用了什么 NSS API 获取智能卡设备。
- 这玩意要一个什么 system shared certificate NSS database。
- 除了 Red Hat 家那一套好像没什么别的发行版弄这个。

这一路下来乱七八糟的名词已经够多的了现在又多了一个什么 NSS 而且只有 Red Hat 才配置了 system shared certificate NSS database，但不管怎么样我是职业选手我不能轻言放弃，还好 Arch Wiki 有这么一个页面 [Network Security Services](https://wiki.archlinux.org/title/Network_Security_Services)，但这不是管证书的吗，和智能卡设备有什么关系啊。这时候我又翻开了 Arch Wiki 关于智能卡的页面 [Smartcards](https://wiki.archlinux.org/title/Smartcards#Chromium)，里面讲了在 Chromium 里面加载智能卡需要在 NSS 数据库里面加一个模块（什么乱七八糟的），不过它操作的都是用户的家目录下面的数据库，这显然不是 system shared certificate NSS database。然后如果手工执行 `/usr/lib/gsd-smartcard -v`，会发现这玩意尝试读取 `/etc/pki/nssdb` 获取什么智能卡驱动列表，我系统里面根本没这个目录。算了，既然是 Red Hat 搞的东西，我看看他们怎么写的。正好我有个 Fedora 的虚拟机，打开虚拟机一看还真有这个目录，那就运行下面命令看看：

```shell
% modutil -dbdir /etc/pki/nssdb -list
```

结果里面除了默认项还真有个叫 `p11-kit-proxy` 的玩意，我又回去看了一眼那个 Merge Request，现在我完全明白了，不知道为什么 NSS 这玩意会记录一个读取智能卡的驱动列表，然后 `gsd-smartcard` 是通过 NSS 获取到智能卡的驱动列表之后再尝试查询智能卡，实际上现在没什么人用 NSS 这个功能了，你这还得往系统的 NSS 数据库里面写东西，除了红帽子家都没人搞这个了，就算有用 NSS 读的（比如浏览器）也是读用户的 NSS 数据库。别的用智能卡的都直接用 `p11-kit` 去读智能卡，所以这个 Merge Request 也改成直接用 `p11-kit` 读了。不知道为什么这个 Merge Request 没能合并。再多说一句，就算是 Red Hat 的系统 NSS 数据库，现在也不直接写智能卡的驱动了，而也是通过 `p11-kit`，所以刚才在 Fedora 的数据库里只看到 `p11-kit-proxy` 这一个驱动……

既然这样我们也在这个数据库里写一个 `p11-kit-proxy`，根据 Arch Wiki 的智能卡页面，如果你要通过 `p11-kit` 操作 OpenSC 的驱动（这都什么乱七八糟的），那可能需要安装一个 AUR 包来保证它被加载（实际上就是个文件而已）：

```shell
% paru opensc-p11-kit-module
```

创建数据库目录并往数据库里写 `p11-kit-proxy`：

```shell
# mkdir /etc/pki/nssdb
# modutil -dbdir sql:/etc/pki/nssdb -add "p11-kit-proxy" -libfile p11-kit-proxy.so
```

如果你和我一样又心急火燎的重启了，就会发现还是没用。这不科学啊，Fedora 的数据库里也是这么写的，看一眼 D-Bus 为什么还是没有智能卡。

实际上最后我发现只差一点点，Fedora 给这个目录下文件的权限是 `-rw-r--r--`，而我这边创建好的是 `-rw------`。`gsd-smartcard` 是以 session 用户运行的当然读不了。所以改一下权限就可以了。

```shell
# chmod 0644 /etc/pki/nssdb/*
```

-------

接下来插着 YubiKey 重启，GDM 启动的 `gsd-smartcard` 就能检测到智能卡，于是调用 `/etc/pam.d/gdm-smartcard`，直接让你输入用户名，输入之后会提示你输入智能卡的 PIN，然后 `pam_pkcs11.so` 进行验证，就可以登录了。锁屏之后也只要输入智能卡的 PIN 就可以解锁。

~~按理说如果给 `pam_pkcs11.so` 发一个空白的用户名，它会根据智能卡返回用户名的，不知道为什么我在 GDM 用不了，一定要开机手动输入，有空我看看代码也许可以修改一下。~~ 我也不知道为什么一定要在 GDM 启动之前插入卡才可以，显示用户列表之后再插入卡我这里没反应。

更新（2022-11-30）：花了我半天时间研究 GDM 和 PAM，问题不在 GDM，而是因为 Arch Linux 的 `gdm-smartcard` 首先调用了 `pam_shells` 检查用户是否有合法的 shell，遇到空用户名它第一个失败了，于是我提交了 [一个 MR](https://gitlab.gnome.org/GNOME/gdm/-/merge_requests/193)，把 `pam_shells` 挪到 `pam_pkcs11` 下面，这样它会检查自动返回的用户名。（虽然这些 PAM 配置文件是发行版自己写的但是大家都提交到 GNOME 那边了，我只改了 Arch 的因为我在用，别的发行版的用户先偷着乐吧。）

如果你想用智能卡解锁的话，一定得是用智能卡登录才可以，它会检查当前的卡是不是登录所用的那张卡，不是的话就只能密码解锁了。折腾这一套花了我一整天时间，因为资料实在是太少了，根本不知道它是怎么工作的。

以及最后我还发现一篇文章，里面的内容也是讲这个 NSS 数据库的解决方案的，也许我早看见这个就不会这么麻烦了： [Fixing NSS and p11-kit in Fedora (and beyond)](https://p11-glue.freedesktop.narkive.com/4z6daFWc/fixing-nss-and-p11-kit-in-fedora-and-beyond)。
