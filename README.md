# Taurix
Taurix OS kernel. Taurix 系统内核，操作系统原理实(xjb)践(写)

Taurix kernel 是一个采用微内核构架，跨平台（但是目前只做了i386(⊙﹏⊙)b，内核提供的接口都是平台与实现无关的），高度模块化的系统内核。

Taurix 最终目标并非玩具操作系统（这样的操作系统已经不少了，照着《30天自制操作系统》糊一个就能弄出来，我估计我一天就能搞定它前15天的进度 <- 无视吐槽），而会致力于实现工业级标准（例如opengl/vulkan图形API，FAT32/EXT4文件系统，网络 等），实现友好，美观的图形化交互界面。

（更详细的说明设计等我做完文件系统再写个文档 ）

# 怎么跑起来?

## Release版 

如果发布了有Release的镜像，在虚拟机中运行镜像即可（镜像作为RAW格式的虚拟磁盘）

## 对于开发者

工具链： 

<table>
<th> <th> Linux <th> Windows <th> Mac OS X
<tr>
<td> cc/cxx/make <td> gcc/g++ <td> Toolchain包中提供MinGW <td> clang/clang++ 待测试
</tr>
<tr>
<td> nasm <td> 包管理器安装 <td> Toolchain包中提供 <td> 包管理器安装
</tr>
<tr>
<td> qemu <td> 包管理器安装 <td> Toolchain包中提供 <td> 待测试
</tr>
<tr>
<td> ruby <td> 包管理器安装 <td> Toolchain包中提供 <td> 系统自带
</tr>
<tr>
<td> PyQt5 on Py3 <td> pip安装  <td> 需自行下载安装 <td> pip安装
</tr>
</table>

其中Python3+PyQt5是可选的，如果没有会导致图形化内核参数设置程序无法运行，将自动使用默认参数

Windows上的开发者的Toolchain包下载：<a href="https://pan.baidu.com/s/11_YUA1vqKfbiQyC1Oo3h6w">百度网盘</a> 进入文件夹后选择针对目标平台(指的是Taurix运行的平台)的压缩包。工具本身的运行需要64位Windows 7及以上支持。

## 运行

克隆仓库并进入：
```
git clone git@github.com:sxysxy/Taurix.git
cd Taurix
```

运行configure程序根据配置生成Makefile，可以使用参数--arch=指定目标平台:
```
ruby configure.rb
```
如果安装的有Python3+PyQt5，且没有进行过内核参数的设置，那么会弹出设置界面进行相关设定。一般可以直接点击SaveExit保存退出，在目录下生成taurix-settings-final.json即可。
- 注意：如果添加了新的源文件，需要重新执行configure.rb以生成Makefile

构建镜像Taurix.img
```
make Image
```

在qemu虚拟机中运行，这一步也会自动执行make Image
```
ruby QemuRun.rb
```

在qemu虚拟机中调试
```
ruby QemuDebug.rb
```
如果要进行调试，会发现qemu虚拟机启动后立刻暂停，这时候就可以使用gdb连接qemu进行调试了。这时可以新打开终端切换到Taurix目录，执行
```
gdb Taurix
target remote localhost:1234
```
就可以进行源码级调试了


