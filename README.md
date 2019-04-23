## Taurix
Taurix OS kernel. Taurix 系统内核，操作系统原理实(xjb)践(写)

## 怎么跑起来?

# Release版 

如果发布了有Release的镜像，在虚拟机中运行镜像即可

# 对于开发者

Windows系统用户:

请使用windows10系统，并安装WSL(Windows subsystem )

- 下载下来这里的源代码，然后[下载工具链](https://github.com/sxysxy/Taurix/releases/download/v0.1/toolchain.7z)，解压到源代码的顶层目录(与src目录同级)。

- 打开 BuildCmd.bat，运行命令
```
rake run
```
即可

Mac OS X / 其它*nix发行版:

- 下载源代码，安装gcc, bochs, ruby, nasm。然后执行命令
```
rake run
```

