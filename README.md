# gxde-deb-installer

# 架构设计

## 前端

主窗口为 DebInstaller 类，前端主页面为 `FileChooseWidget`，它是一个文件选择界面，当用户选择了一个/多个 deb 文件后，进入后续的安装界面。

> 在用户选择了软件包后，应该对 deb 包进行格式效验，以过滤那些 Invalid 的文件。

单个安装界面与批量安装界面是可以互相转换的，如果在单软件包界面继续拖放更多的软件包，即跳转到多软件包安装界面。而如果在批量安装界面将软件包一个个删除，当安装列表只剩一个软件包时，则自动跳转到单软件包安装界面。

由于这些界面转换的需求，在单/多软件包安装界面层上，只是一个选择性的数据显示，真正的等待安装的软件包列表保存在 `DebListModel` 类中。

### 单软件包安装界面

当用户只选择了一个软件包后，进入单软件包安装界面。如果继续向里面添加软件包，则跳转到多软件包安装界面。

### 批量安装界面

当用户选择了一个以上的软件包后，进入批量安装界面。当用户删除比量安装列表至最后一个时，跳转到单软件包安装界面。

批量安装界面使用 Qt Model-View 设计模式，`DebListModel` 作为数据层，`PackagesListDelegate` 作为绘画的代理层，而 `PackageListView` 为表示层。通过这个模式，可以在不对 `DebListModel` 做过多侵入式设计的前提下，实现数据的获取。

### 卸载确认界面

这是一个在执行卸载命令之前，用来让用户确认所卸载的包列表的界面。

## 控制端

`DebListModel` 是联系前/后端并提供查询、安装、卸载命令等最主要最核心的类。对上，它提供了软件包列表的各种数据；对下，它封装了具体的安装、卸载等操作。

利用 `PackagesManager` 类所提供的数据，向界面上显示对应的软件包信息。通过封装 `libqapt` 的查询与安装接口，提供统一的 `Transaction` 类来回报进度等信息。

## 后端

### PackagesManager

`PackagesManager` 类是主要的 deb 包管理类，他使用 `libqapt` 来进行 deb 包格式的读取与解析。

#### 依赖解析
`PackagesManager` 类另一个核心功能，也是 `gxde-deb-installer` 项目最重大的问题，就是软件包的依赖解析。在本类中，实现了一个对 deb 包的依赖解析流程。

#### 依赖解析结果

依赖解析的结果分为 3 种，分别是：

- 依赖冲突，不可安装此软件包
- 依赖可用，安装或升级某些依赖包后可以安装此软件包
- 依赖满足，可以直接安装此软件包

#### 不同种类的依赖解析

此外，在依赖解析的过程中，要考虑可升级/降级依赖、间接依赖、循环依赖、虚包、Providers 包等问题：

- 可升级依赖：当前依赖不满足条件，但仓库中此包的新版本可用，可以升级到仓库中的版本来解决依赖。
- 间接依赖：当前第一级依赖都满足条件，但其中一个依赖包的依赖关系不满足，此时也无法安装。即，所有依赖关系的查找必须查找到基础包为止，才能发现所有的间接依赖。
- 循环依赖：在查找依赖链时，有可能出现 A 依赖 B 而 B 也依赖 A 的情况，程序应该要有有效的机制避免循环依赖带来的问题。
- 虚包：当发现依赖包是一个虚包时，应该查找它由哪些包所填实，并依次解决它们的依赖关系。
- Providers：例如某些程序依赖 java，而 java 是由 jdk 或 jre 提供的子包，并不单独存在 java 这个包。在此时，就应该遍历仓库，查找包的 Providers 并比对相关信息，以正确的安装对应的依赖。

#### 多架构问题

另外，还要解决多架构及跨架构依赖带来的问题：

这个问题主要发生在 x86 64Bits 系统下，系统中可能同时存在 x86 32Bits、x86 64Bits 两个包架构。

- 一般来说，没有特别指明的情况下，都安装默认的架构（即 64Bits）。
- 在安装依赖时，安装与本包相同的架构。即本包是 32Bits，就安装 32Bits 的依赖包。
- 要特殊处理的是 AnyArch 及 NativeArch 的相关问题
	- 某些包是架构不相关或不敏感的（例如一些 Python 库），此时可以跨架构解决依赖，要解析对应的字段来完成这个判断。

> 当多架构依赖与之前的各种依赖解决方案混合时，问题变得比较复杂，这部分要做仔细的 code-review 和测试。


#### 总结

区别与其它的 deb 安装器，`gxde-deb-installer` 支持批量安装。在考虑问题的过程中，可以以单个应用程序安装过程为例，批量安装仅仅是对单个程序安装的重复操作。但是在设计数据结构及编写具体逻辑时，要考虑多应用同时安装的情况。

由于 deb 软件包的复杂性，对其依赖进行解析并不是一件简单的事。在不同的策略、不同的解析顺序下，可能有多种解决依赖关系的可能。所以，这种不确定性可能会导致用户的一些困惑（即与其它安装器的依赖解析结果不相同）。而如何判断是程序出了 bug，还是这是一个正常的依赖解析结果，就需要对 deb 包及软件仓库有更多了解。

在批量安装时，每安装完成一个软件包，都应当对仓库状态、依赖状态进行重新刷新与解析。因为在某些包安装后，系统环境中的依赖关系可能已经与之前不同了。

> 即便是 apt 和 gdebi 这两个最常用到的 deb 安装程序，在安装某些包时，解决依赖的方法都不一定完全相同。

> 将来，可以考虑换用其它库进行依赖解析。自己实现依赖解析既复杂、难维护，又容易出 bug，解析行为上也难以和 apt 保持一致。

#### 依赖解析示例

以下举几个比较经典的例子：

##### 解析示例 1

- A -> B | C	# 软件包 A 依赖软件包 B 或者软件包 C
- B -> D(v1.0)	# 软件包 B 依赖包 D 的 v1.0 版本
- C -> D(v2.0)	# 软件包 C 依赖包 D 的 v2.0 版本
- 已安装 D(v1.0)	# 系统中已经安装了包 D 的 v1.0 版本
- 可升级 D(v2.0)	# 仓库中有包 D 的 v2.0 版本

在这种情况下，当安装软件包 A 时，就有两种方法可以解决依赖。

1. 安装软件包 B。
2. 安装软件包 C，并升级软件包 D 到仓库中的 v2.0 版本。

这两种方法都是正确的并可以满足软件包 A 的依赖需求。但是要注意在第 2 种解决方案下，要检查一下软件包 D 由 v1.0 升级到 v2.0 是否会引入冲突等问题。

# 缺陷

由于安装器完全使用自己的依赖解析逻辑。并且调用了 `libqapt` 进行软件包安装（其底层是包装 `dpkg -i` 命令执行的安装），与 apt 不同的是，这样做 apt 是无法知道哪些包是用户主动安装的(即用户主动拖动到列表中执行安装)，哪些是作为依赖被动安装的。这样，以后在卸载某个包是，当初作为它的依赖被安装上的包就无法使用 autoremove 被卸载了（因为所有的包在 apt 看来都是主动安装的）。

即使不改变目前的依赖解析及软件包安装逻辑，这个问题也是可以解决的。在 apt 中修改包的属性，将它的安装原因改为依赖安装即可，不过目前还没有实现。

# libqapt

## 简介

libqapt 是 qapt 的一个库，qapt 是像 gdebi 一样的一个 deb 安装程序。它底层使用了 `libapt-pkg` 进行一些仓库的访问。本身实现了以调用 dpkg 命令为基础的软件包安装逻辑。

libqapt-runtime 是这个库中实现权限操作的后端，它会注册一个 system-dbus 提供服务。libqapt 中相应的权限操作都通过 RPC 方式与此后端通信。

## Debug

libqapt 本身是一个 C++ 项目，很容易编译安装，可以直接下载源码通过加日志、GDB 等方式进行调试。要注意的是可能需要在调试时修改代码禁止或者放宽某些超时操作。在调试 runtime 的时候要确保当前运行的不是旧版本 runtime。

## 为 libqapt 打补丁

在开发过程中，发现了数个 `libqapt` 的 bug。为了快速解决问题，现在已经给 `libqapt` 打了以下几个补丁，在上游的新版本推送后，应该积极维护这些补丁列表。

- 0001-add-zh_CN-translate.patch 添加中文翻译，hack 的做法。
- 0001-fix-long-description-error.patch 修复 libqapt 在解析 deb 包时，对某些不是特别规范的 control 文件解析出错。
- 0001-fix-old-error-not-clear.patch 修复 libqapt 在重复使用 dpkg 命令时，相应的类没有清理旧错误信息。
- ~~0001-Fix-install-transaction-timeout.patch 修复 libqapt 在安装时超时信息设置错误导致安装失败。~~ __上游已经合并__

# 参考资料
- libqapt
	- [sources](https://github.com/KDE/libqapt)
	- [online doc](https://api.kde.org/extragear-api/sysadmin-apidocs/libqapt/html/index.html)
