---
title: 构建和运行 Xen HVM 和 PV
layout: post
#comment: true
created: 2024-03-07T23:19:19
updated: 2024-03-12T17:59:48
categories:
  - 计算机
  - Linux
tags:
  - 虚拟化
  - Xen
---
<div class="alert-red">请注意本文并不是推荐读者使用 Xen 作为虚拟化方案，相反，KVM 才是目前更合适大部分读者的方案。</div>

# 简介

由于工作需要，最近我需要搞一个 Xen PV 来进行测试，在此之前我一直使用 qemu/KVM，只是听说 Xen 是 KVM 之前流行过的虚拟化方案。比起几乎什么都不需要做交给 libvirt 包办就可以的 KVM，Xen 的设置相对要复杂一点。

<!--more-->

首先 Xen 分为 HVM 和 PV 两种常见的虚拟方案（PVH 我也没用过），HVM 依赖于硬件虚拟化，和常见的虚拟机没什么区别，而 PV 并不依赖硬件虚拟化，是通过虚拟机 Linux 内核中特殊的驱动，将请求转给宿主机的内核代为操作，但配置起来也更加复杂。目前 PV 所需的代码已经并入 Linux 内核上游，你能安装的发行版大部分都可以直接作为 PV 的虚拟机运行。

无论是 HVM 还是 PV，Xen 都用 domU（domain 的缩写，U 可以是 1、2、3……）代表虚拟机，然后用 dom0 代表宿主机，当然文档里也会叫宿主机 Hypervisor，这里我也可能直接将虚拟机叫做 VM。

# 构建和运行 dom0

<div class="alert-blue">温馨提示：根据我的经验，在 Xen Hypervsior 上运行的 GNOME 桌面会由于未知原因在未知操作时卡住，建议不要用你平时使用的开发机作为 Xen Hypervisor，而是另找一台机器作为服务器运行 Xen Hypervisor，然后远程连接上去操作。</div>

和 KVM 不需要什么操作就能用不一样，Xen 需要你构建一个单独的 boot loader，在加载 Linux 内核之前先加载它，从而实现 Xen 的支持。我这里用的 Arch Linux，需要构建 `xen` 这个 AUR 包，根据 wiki 所说推荐用下面的指令构建：

```shell
$ build_stubdom=true efi_dir="/boot" makepkg -si
```

我这里直接把 ESP 挂载到 `/boot`，你可能需要按情况修改。

你还需要 `xen-qemu` 这个 AUR 包提供 qemu 前端对 Xen 的支持，但和一般的 AUR 包不同的是如果你直接构建这个包，得到的包很可能和你已经安装的 qemu 文件冲突。这是我遇到的唯一一个必须在 clean chroot 才能构建的包。然后由于它依赖 `xen` 这个 AUR 包，你必须手动操作。

首先安装 `devtools`：

```shell
# pacman -S devtools
```

然后创建一个用于构建 clean chroot 的目录：

```shell
$ mkdir ~/chroot
```

然后在里面安装基础依赖：

```shell
$ mkarchroot ~/chroot/root base-devel
```

一般这时候 chroot 里面应该已经是最新的了，但也可以用下面的命令更新：

```shell
$ arch-nspawn ~/chroot/root pacman -Syu
```

然后切换到你包含 `xen-qemu` 的 `PKGBUILD` 的目录，你可能需要的两个依赖是 `xen` 和 `numactl`，前者我们刚刚构建过，后者我们可以直接从官方仓库通过 `pacman -S numactl` 安装，然后在命令行参数里指定这两个文件的位置：

```shell
$ makechrootpkg -c -r ~/chroot -I ../xen/xen-4.18.1pre-1-x86_64.pkg.tar.zst -I /var/cache/pacman/pkg/numactl-2.0.18-1-x86_64.pkg.tar.zst
```

然后用 `pacman -U xen-qemu-*.tar.pkg.zst` 安装你刚刚构建好的包，此时应该没有文件冲突了。

你还需要安装下面的包提供虚拟机内的 BIOS 和 UEFI 引导支持：

```shell
# pacman -S seabios edk2-ovmf
```

然后你需要加载构建好的 Xen boot loader，让它在 Linux 内核之前启动，我使用的 systemd-boot，所以下面就简单写 systemd-boot 的配置方式，逻辑上是完全一致的，如果你使用 GRUB，建议参考 Arch Wiki。

首先添加一个 systemd-boot 启动项文件，我这里使用 `/boot/loader/entries/xen.conf`：

```conf
title Xen Hypervisor
sort-key xen
efi /xen.efi
```

如果你也用 systemd-boot，这个文件对你来说应该非常简单，构建 `xen` 包的时候已经将支持 EFI 的 Xen boot loader 也就是 `xen.efi` 这个文件安装到了 ESP，只要引导它就可以。

接下来我们编写 Xen 的配置文件让它可以正确找到你的 initramfs 和内核，并传递内核参数，配置文件 `xen.cfg` 需要和 `xen.efi` 位于同一个目录，这里就是 `/boot/xen.cfg`：

```conf
[global]
default=xen

[xen]
options=console=vga loglvl=all noreboot
kernel=vmlinuz-linux root="UUID=XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX" rootfstype="btrfs" rootflags="rw,defaults,noatime,compress=zstd:3,ssd,space_cache,subvolid=257,subvol=/@" rw add_efi_memmap threadirqs nvidia_drm.modeset=1
ramdisk=initramfs-linux.img
ucode=amd-ucode.img
```

以上的内核参数是我所用的，你可以从你当前的 boot loader 启动项文件里复制出你正在用的内核参数。如果你复制了 `xen` 包自带的示例文件而不是我的，需要注意里面包含限制宿主机可用内存的参数，这是为了避免在创建虚拟机时再限制宿主机内存，影响宿主机各种缓存的策略，默认的值给的很小，可能导致无法正常启动桌面，你可能需要改大一点。不过对于我们这种简单 debug 用，可以直接忽略此参数，影响不大。

然后你需要让 Xen 所需的一些守护进程开机启动：

```shell
# systemctl enable xenconsoled.service xen-init-dom0.service xen-qemu-dom0-disk-backend.service xendomains.service
```

然后重启系统，选择名字是 `Xen Hypervisor` 的启动项，你应该就有一个可以运行 Xen 的环境了。

# 配置和运行 domU

然后按道理说既然 Xen 和 KVM 都是用 qemu 作为前端，那完全可以交给我常用的 virt-manager 操办一切，但我尝试构建了带 Xen 支持的 libvirt，结果运行起来发现由于某个 bug，它并不能真的支持 Xen，所以只能全手动操作了。

## 构建虚拟机用的 NAT 网络

virt-manager 会帮我自动创建一个 NAT 网络使得虚拟机之间可以互相联系并且通过宿主机访问外网，我相信大部分人都需要让虚拟机联网，但只能我自己解决这件事了。

具体的操作包含以下几步：添加一个桥接接口；然后给它分配一个 IP；再开启系统的 NAT 转发（包含 `sysctl` 和 `iptables` 两部分，我自己也不是 `iptables` 高手所以我也不能给你解释）；然后在这个接口上启动 DHCP 服务器，给虚拟机提供 IP 地址和 DNS 服务器。这样就完成了宿主机的部分，Xen 可以根据虚拟机的配置文件自动把虚拟机添加到你刚才创建的桥接接口上。

为了简化这个操作我编写了一个脚本：

```bash
#!/bin/bash

set -x

OUT_IF="${1}"
BR_IF="${2}"
BR_IP="192.168.123.1"
BR_IP_RANGE="192.168.123.100,192.168.123.200"
BR_DNS="1.1.1.1,8.8.8.8"

[[ -z "${OUT_IF}" ]] && exit 1
[[ -z "${BR_IF}" ]] && BR_IF="vmbr0"

ip link add name "${BR_IF}" type bridge
ip link set dev "${BR_IF}" up

ip address add dev "${BR_IF}" "${BR_IP}/24"

# Enable NAT, so VMs can accept Internet.
FORWARD=$(sysctl --values net.ipv4.ip_forward)
# See <https://www.karlrupp.net/en/computer/nat_tutorial>.
sysctl net.ipv4.ip_forward=1
iptables -t nat -A POSTROUTING -o "${OUT_IF}" -j MASQUERADE

# Start DHCP server so we don't need to manually assign IP addresses for VMs.
# `--no-daemon` starts dnsmasq in debug mode, so it won't overload SIGINT.
# It is hard to prevent `dnsmasq` from listening `127.0.0.1:53` as a DNS server,
# so just disable its DNS server, and send DNS server to VMs via DHCP.
# It seems you need to set `--bind-interfaces` with `--interface` to make it
# bind to a interface only, otherwise it will try to bind to other interfaces
# and conflict with other dnsmasq instance (if exists).
dnsmasq --no-daemon \
	--port=0 \
	--interface="${BR_IF}" \
	--bind-interfaces \
	--dhcp-range="${BR_IP_RANGE}" \
	--dhcp-option="option:dns-server,${BR_DNS}"

iptables -t nat -D POSTROUTING -o "${OUT_IF}" -j MASQUERADE
sysctl "net.ipv4.ip_forward=${FORWARD}"

ip link set dev "${BR_IF}" down
ip link delete "${BR_IF}"
```

你可以把这个脚本保存成 `mkvmbr0.sh`，然后 `chmod +x mkvmbr0.sh`，然后用 `ip a` 查看你当前联网所用的端口名，比如我的是 `wlp5s0`，就可以 `sudo ./mkvmbr0.sh wlp5s0` 创建一个运行在 `vmbr0` 端口上的 NAT 网络，脚本会启动 `dnsmasq` 作为 DHCP 服务器并通过 DHCP 服务器给虚拟机下发 DNS 服务器地址，如果你已经用完了虚拟机，`Ctrl+C` 打断 `dnsmasq` 它就会进行后续的清理工作并退出。

## 编写 HVM 配置文件

HVM 实在是没什么复杂的，这么说是相对于 PV 而言，比如说构建一个 openSUSE Tumbleweed 的 HVM 可以写一个叫做 `hvm-tumbleweed.cfg` 的文件：

```conf
name = 'hvm-tumbleweed'
builder = 'hvm'
memory = 2048
vcpus = 4
disk = [ 'file:/home/alynx/xen/disk-tumbleweed.img,xvda,rw', 'file:/home/alynx/xen/openSUSE-Tumbleweed-DVD-x86_64-Current.iso,sdb:cdrom,r' ]
vif = [ 'mac=00:16:3e:00:00:02,bridge=vmbr0' ]
vnc = 1
vnclisten = '0.0.0.0'
vncdisplay = 1
```

这里没什么要注意的，无非是 `bridge=` 后面接你 NAT 网络的桥接端口。以及你在配置文件里写 `xvda` 在虚拟机里会变成 `sda`，所以不要再写另一个叫做 `sda` 的设备了。如果你有多个虚拟机，记得修改 MAC 地址和 VNC 端口。

创建磁盘文件可以用下面的命令：

```shell
$ truncate -s 20G disk-tumbleweed.img
```

然后用下面的命令就可以启动这个虚拟机：

```shell
# xl create hvm-tumblweed.cfg
```

其他的命令可以直接 `man xl` 查看手册。可以使用 `vncviewer YOUR_HOST_IP:1` 连接虚拟机。关于这个配置文件的具体语法可以参考 [官方文档](https://xenbits.xen.org/docs/unstable/man/xl.cfg.5.html)。

## 编写 PV 配置文件

PV 比起 HVM 可就复杂太多了，最痛苦的一个问题是由于它不是硬件虚拟化，所以你没有办法运行虚拟机磁盘上的 boot loader！解决方案有两个，要么是直接在配置文件里写好内核和 initramfs 的路径（这样你就得想办法把虚拟机的内核和 initramfs 搞到宿主机磁盘上），要么是在宿主机上构建一个 GRUB 镜像，然后让这个 GRUB 去找虚拟机磁盘里的 `grub.cfg` 并执行，然后引导虚拟机里面的内核（这都哪跟哪啊）。

当然，如果你虚拟机的系统并不使用 GRUB 作为 boot loader，那你就只能使用第一种方案了。对于 openSUSE Tumbleweed，我摸索通了后面的方案，因此我在这里介绍这个方案如何操作。

首先你需要克隆 GRUB 的源码，因为你得专门构建一个能在 Xen 虚拟机里运行的 GRUB：

```shell
$ git clone git://git.savannah.gnu.org/grub.git
```

然后构建（如果你的 Xen 是 32 位机器上的，把 `amd64` 换成 `i386`：

```shell
$ ./autogen.sh
$ ./configure --prefix=/opt/grub-xen --target=amd64 --with-platform=xen
$ make
# make install
```

这会将 Xen 版本的 GRUB 安装到 `/opt/grub-xen`，接下来我们需要利用 GRUB 可以将一个 tar 作为 memdisk 的特性，在里面写一个 `grub.cfg` 让 GRUB 去按我们指定的路径搜索实际的 `grub.cfg` 并加载它（别问我为什么，这鬼东西简直太邪门了）。

首先写一个 `grub-bootstrap.cfg`，这个文件的唯一作用就是让 GRUB 加载 memdisk 里面的 `grub.cfg`：

```
normal (memdisk)/grub.cfg
```

然后写一个 `grub.cfg`，下面关键的问题来了，你怎么知道要搜索的真正的 `grub.cfg` 的路径呢，当然是想办法挂载出来自己看了，我这个文件里写了常见的安装好的系统的 `grub.cfg` 的位置和我自己看到的 openSUSE 安装 iso 里面的 `grub.cfg` 的位置，所以可以同时支持安装和启动系统：

```
if search -s -f /boot/grub/grub.cfg ; then
    echo "Reading (${root})/boot/grub/grub.cfg"
    configfile /boot/grub/grub.cfg
fi

if search -s -f /boot/grub2/grub.cfg ; then
    echo "Reading (${root})/boot/grub2/grub.cfg"
    configfile /boot/grub2/grub.cfg
fi

if search -s -f /grub/grub.cfg ; then
    echo "Reading (${root})/grub/grub.cfg"
    configfile /grub/grub.cfg
fi

if search -s -f /grub2/grub.cfg ; then
    echo "Reading (${root})/grub2/grub.cfg"
    configfile /grub2/grub.cfg
fi

if search -s -f /EFI/BOOT/grub.cfg ; then
    echo "Reading (${root})/EFI/BOOT/grub.cfg"
    configfile /EFI/BOOT/grub.cfg
fi
```

然后把它打包成 tar：

```shell
$ tar -cf memdisk.tar grub.cfg
```

然后创建一个支持 Xen 的包含所有 GRUB 模块的 GRUB 镜像，我们将把它当作真正的虚拟机的 boot loader 运行，使用如下命令：

```shell
$ /opt/grub-xen/bin/grub-mkimage -O x86_64-xen \
       -c grub-bootstrap.cfg \ 
       -m memdisk.tar \
       -o grub-x86_64-xen.bin \
       /opt/grub-xen/lib/grub/x86_64-xen/*.mod
```

然后你就可以编写一个 `pv-tumbleweed.cfg` 的配置文件：

```
name = 'pv-tumbleweed'
memory = 2048
vcpus = 4
kernel = "grub-x86_64-xen.bin"
disk = [ 'file:/home/alynx/xen/disk-tumbleweed.img,sda,rw', 'file:/home/alynx/xen/openSUSE-Tumbleweed-DVD-x86_64-Current.iso,sdb:cdrom,r' ]
vif = [ 'mac=00:16:3e:00:00:01,bridge=vmbr0' ]
vnc = 1
vnclisten = '0.0.0.0'
vncdisplay = 1
```

没错我们这里把我们刚刚生成的 boot loader 作为内核首先拉起来，然后不出意外你应该就能看到安装程序启动了，但是如果你火急火燎的一路下一步安装，你就会掉进下一个坑：openSUSE 默认使用 snapper 管理 btrfs 快照，默认的配置方案把 `/boot` 也放在 btrfs 子卷上。而上游的 GRUB 会从 btrfs 的根子卷而不是默认子卷开始访问，我是没能搞清楚该如何简单直接的访问 snapper 最新的快照所在的子卷，openSUSE 的 GRUB 则是打了一大堆 patch 让 GRUB 支持查找和加载默认的 btrfs 子卷。总之你直接安装之后我们的 `grub.cfg` 是查找不到真正的 `grub.cfg` 的，最简单的方案就是安装时干脆不要用 btrfs 从而不用 snapper，或者把 `/boot` 单独分区单独格式化单独挂载。

然后你应该可以用下面的命令启动并链接到虚拟机的终端了：

```shell
# xl create -c pv-tumblweed.cfg
```

关于这个配置文件的具体语法可以参考 [官方文档](https://xenbits.xen.org/docs/unstable/man/xl.cfg.5.html)。

# 这一切值得吗？

在经历这一系列不知道是什么鬼东西的操作之后你终于有了一个可以用的 Xen PV，也许它唯一的优势就是可以在没有硬件虚拟化的机器上跑虚拟机，为此你付出的代价是一个没法用桌面的宿主机，一个说不定哪个新系统就没法引导的虚拟机 boot loader。但现在的设备有几个没有硬件虚拟化支持呢？这就意味着对于大多数人你可以简单地使用 qemu/KVM 几乎不需要任何额外的配置，并且还有 virt-manager 这样的程序全程帮你图形化配置虚拟机和 NAT 网络。

而比如你想用上面的办法手动构建网络并启动一些 SLES Minimal OS 或者 openSUSE JeOS，这个过程也更简单，首先在 `/etc/qemu/bridge.conf` 里加入一行 `allow vmbr0`，然后用之前的 NAT 网络脚本 `sudo ./mkvmbr0.sh wlp5s0`，再用下面的 qemu 命令：

```shell
$ qemu-system-x86_64 \
    -enable-kvm \
    -m 1G \
    -smp 1 \
    -drive if=virtio,format=qcow2,file=SLES15-SP5-Minimal-VM.x86_64-kvm-and-xen-GM.qcow2 \
    -nographic \
    -netdev bridge,id=eth0,br=vmbr0 \
    -device virtio-net,netdev=eth0
```

它会调用 `qemu-bridge-helper` 自动将虚拟机加入你构建的桥接 NAT 网络，和 Xen 比起来简单很多，也不需要额外的配置。因此如果你只是需要一个自己的虚拟化平台，完全没有必要使用已经不再流行且可能存在更多问题的 Xen，使用 KVM 就足够了。这篇文章仅仅是为了在读者不得不需要构建一个 Xen 环境 debug 时作为参考。
