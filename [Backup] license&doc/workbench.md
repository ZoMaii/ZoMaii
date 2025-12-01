# Maic WorkBehance
Maic WorkBehance 是一个架构，也是 MaicQCR 的构造原理，它的结构如下：

```txt
-- 这是总体架构
Frame(Private, General, Pubilc)

-- 这是分支架构
Private(Frame<Private>, Query<Private>, API<Private>)
General(Query<Private>, Package<public>)
Public(License, Agreement, open-standard)
```

通俗来说，在开发时应用了此框架，对开发者而言：
 - 如果 **你关注系统**，使用 `Private` 级的命令即可
 - 如果 **你关注数据共享、分发**，使用 `General` 级的命令即可
 - 如果 **你关注指定系统下的数据共享、分发** 使用 `Private.General` 即可
 - 如果 **你关注某些技术、应用**，使用 `Public` 即可

我们认为的 **最优实践原则** 如下：
 - 使用原生系统、提供商支持的命令完成业务，以确保代码健硕、快速
 - 如果原生系统提供的命令不利于维护、编写，则应该把原生命令列入观察候选！
 - 附加的 `General` 级命令标准以开发者为用户、使用为中心。而非设备持有者

在任何情况下，它的辨别拼写都是 `[产品/标识名].[分支名]` ！


## 如何理解 Private 级别
Private 级别与 **系统（BIOS）**、**硬件服务（Hardware）** 之间硬绑定，也是我们在设计之初时考虑的原稿。

正如我们要 **获取硬盘信息** 需要在 **Linux类** 系统下使用 `lsblk`, **MacOS** 中使用 `diskutil`, **windows** 中连接 **WMI** 工具等情况

**Private** 级别是完全由 **硬件厂商**、**系统提供商** 内部开发定义，因此可能有 **存在多个Frame架构**！我们提供的 Private命令除了 `Private.General` 之外，没有任何定义，都是直接连接的提供商服务！


## 如何理解 General 级别
General 级别也不是由我们单独规定的，在绝大部分情况下是由你的 **设备制造商**、**系统提供商** 所决定的。

这两位提供商可以提供 **平台服务**(Paas) 内容，但输出的标准可能并不一致，但这些 **正常的提供商一定会提供一份开发标准**，我们将这些文件统称为 **是 General** 级别。

默认情况下，你使用的 **General** 级别命令，是 **由我们直接提供的标准化输出结果**！如果你认为你的服务商提供了此命令，请使用 `Private.General` 级命令！遵循我们的 **最优实践原则**！

我们 **不会构造** 任何有明显 **特殊定制** 业务倾向的 `General` 代码命令!

## 如何理解 Public 级别
Public 级需要手动进行 Publish 操作。它 **不由任何企业单独控制**，所有的 Public 级别都需要 **有主流编程语言的支持**、**说明文档**、**对应的社区支持**。成为 Public 级别意味着无需用 General 引用对应的命令。

对于它的应用也将对应变化为 `Public.General`，例如 `html.firefox`，默认引用的是在业务上广受支持的、社区上广受欢迎的产品！



## 标准化输出参考标准
在任何情况下，我们要求输出的级别格式与 CentOS7 系列输出相符，具体内容可以稍有不同，但焦点元素必须一样！

例如 **获取硬盘信息** ，用户希望获取到什么？是分区还是硬盘本身(大小，使用情况，日期)？我们第一时间是不知晓的！但追随 CentOS 系统要求在 `General` 的输出标准如下：

```txt
# Windows 系统

硬盘：samsung 990 pro 1TB (ID: \\.\xxxx0)
    硬盘大小：931.51 GB
        分区：C: - Windows（450.00 GB）
        分区：D: - Data（100.00 GB）

硬盘：samsung 990 evo 2TB (ID: \\.\xxxx1)
    硬盘大小：1.81 TB
        分区：E: - Backup（1.81 TB）

# Linux系统 - 遵循最优原则使用原生命令
> $ lsblk -o name,size,type,mountpoint

NAME        SIZE    TYPE    MOUNTPOINT
sda         100G    disk
├─sda1      50G     part    /mnt/data
├─sda2      48G     part
└─sda1      2G      part    [SWAP]
sdb         200G    disk
└─sdb1      200G    part    /mnt/backup
```
标准化输出有以下参考原则：
 - 原系统是否存在相关命令，是否符合开发直觉而非业务直觉？
 - 其他系统中是否有对应的输出命令？