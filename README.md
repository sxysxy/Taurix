## Taurix
Taurix OS kernel. Taurix 系统内核，操作系统原理实(xjb)践(写)

Taurix kernel 是一个采用微内核构架，跨平台（但是目前只做了i386(⊙﹏⊙)b，内核提供的接口都是平台与实现无关的），高度模块化的系统内核。

Taurix 最终目标并非玩具操作系统（这样的操作系统已经不少了，照着《30天自制操作系统》糊一个就能弄出来，我估计我一天就能搞定它前15天的进度 <- 无视吐槽），而会致力于实现工业级标准（例如opengl/vulkan图形API，FAT32/EXT4文件系统，网络 等），实现友好，美观的图形化交互界面。

（更详细的说明设计等我做完文件系统再写个文档 ）

## 怎么跑起来?

# Release版 

如果发布了有Release的镜像，在虚拟机中运行镜像即可（镜像作为RAW格式的虚拟磁盘）

# 对于开发者

目前暂时只支持在linux系统上进行开发，我们会尽快完善windows上的开发环境

需要：python3(with pyqt5), ruby, grub2(with grub-mkrescue command), gcc/g++, nasm, make, qemu

克隆仓库并进入：
```
git clone git@github.com:sxysxy/Taurix.git
cd Taurix
```

执行配置：
```
./taurix-settings-gui.py
```
直接点击SaveExit使用默认配置即可

根据配置生成Makefile，可以使用参数--arch=指定目标平台:
```
./configure.rb
```
如果添加了新的源文件请重新执行configure

构建镜像Taurix.img
```
make Image
```

在qemu虚拟机中运行，这一步也会自动执行make Image
```
./QemuRun.sh
```

在qemu虚拟机中调试
```
./QemuDebug.sh
```
如果构建成功，会发现qemu虚拟机启动后立刻暂停，这时候就可以使用gdb连接qemu进行调试了。这时可以新打开终端切换到Taurix目录，执行
```
gdb Taurix
target remote localhost:1234
```
就可以进行源码级调试了


