## Taurix
Taurix OS kernel. Taurix 系统内核，操作系统原理实(xjb)践(写)

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

