#!/usr/bin/ruby
# Generate makefile from settings for building taruix
#   author: hfcloud(sxysxygm@gmail.com) 
#     date: 2019.07.28

require 'json'

SETTINGS_FILENAME = "taurix-settings-final.json"
DIR_ROOT = Dir.pwd

#加载设置
->{STDERR.puts("#{SETTINGS_FILENAME} not found. Please run taurix-settings-gui.py to generate it."); exit 1}[] unless File.exist?(SETTINGS_FILENAME)
SETTINGS = JSON.parse(File.read(SETTINGS_FILENAME))
AVALIABLE_ARCH = SETTINGS["Architecture"].keys

#--目标平台的设定，如果命令行参数不指定则默认i386
$arch = nil
argv = ARGV
while arg=argv.shift
    if arg =~ /--arch=(.+)/
        $arch = $1
    elsif arg == "--help" || arg == "-h"
        print "configure:\n\t--arch=target_architecture, default to i386.\n\tAvaliable architectures:\n\t\t#{AVALIABLE_ARCH.join("\n\t\t")}\n"
    end
end

$arch = "i386" if !$arch
if !AVALIABLE_ARCH.include?($arch)
    puts("Unsupported architecture: #{$arch}")
end

#------------源文件和头文件的目录-------
# 脚本中涉及到源文件的相对路径，都是相对于DIR_SRC的，头文件同理
DIR_SRC = File.join(DIR_ROOT, "src")  
DIR_INCLUDE = File.join(DIR_ROOT, "include")

#----------工具链设定---------
# fixme: 虽然留了多种硬件体系的设置，但是hfcloud太菜了暂时只搞了x86_64
ASM = "nasm"
CC = "gcc"
CXX = "g++"
LINK = "ld"
RM = "rm -f"
  #!! 编译参数设定
  #平台相关的参数（例如位数的指定）在src/arch/$arch/extra_flags.rb中
_CC_FLAGS_COMMON = ["-fno-builtin -fno-stack-protector", "-I./include"]
_CXX_FLAGS_COMMON = ["-fno-builtin -fno-stack-protector ", "-I./include"]
_ASM_FLAGS_COMMON = []
_LINK_FLAGS_COMMON = []  
#加载平台相关的参数
eval File.read((File.join("#{DIR_SRC}", "/arch/#{$arch}/extra_flags.rb")))

if SETTINGS["Debug"]["enable gdb debug"]
    _CC_FLAGS_COMMON.push("-g -O0")
    _CXX_FLAGS_COMMON.push("-g -O0")
    _ASM_FLAGS_COMMON.push("-g")
end

CC_FLAGS_COMMON = _CC_FLAGS_COMMON.join(' ')
CXX_FLAGS_COMMON = _CXX_FLAGS_COMMON.join(' ')
ASM_FLAGS_COMMON = _ASM_FLAGS_COMMON.join(' ')
LINK_FLAGS_COMMON = _LINK_FLAGS_COMMON.join(' ')
#-----------------------------

#------文件依赖表，哈希表的key为文件名（相对路径），value为一个数组，数组中为它所依赖的文件----
SOURCES = {}
C_EXTNAMES = [".c"]
CXX_EXTNAMES = [".cpp", ".cc"]
ASM_EXTNAMES = [".S", ".asm"]

C_CXX_EXTNAMES = C_EXTNAMES + CXX_EXTNAMES
SRC_EXTNAMS = C_CXX_EXTNAMES + ASM_EXTNAMES
#----------------------------

def scan_file_dependence(file, recoder)
    ext = File.extname(file)
    #排除掉不是源代码文件
    return false unless SRC_EXTNAMS.include? ext
    
    fp = File.open(file)
    recoder[file] = []
    fp.each_line do |line|
        if C_CXX_EXTNAMES.include?(ext)
            /#include\s*[<"](.+)[>"]/.match(line)
            recoder[file].push(File.join("./include", $1)) if $1
        #else ASM_EXTNAMES.include?(ext)
        end
    end
    fp.close
    return true
end

def scanf_files(stack)
    cur_dir = stack.join('/')
    if(stack[-1] == "arch") #如果是在arch文件夹中，只扫描配置指定的平台的源文件
        stack.push("#{$arch}")
        scanf_files(stack)
        stack.pop
        return 
    end
    Dir.foreach(cur_dir) do |f|
        next if f == "." || f == ".."
        subfile = File.join(cur_dir, f)
        if File.directory?(subfile)
            stack.push(f)
            scanf_files(stack)
            stack.pop
        else 
            scan_file_dependence(subfile, SOURCES) 
        end
    end
end

#源文件对应的目标文件名
def src_obj_file(file)
    base = File.basename(file)
    i = base.length-1
    i -= 1 while base[i] != "."
    return base[0, i]+".o"
end

#扫描源文件，生成对应的目标文件
scanf_files(["./src"])
OBJECTS = []
SOURCES.each_key do |k|
    OBJECTS.push(src_obj_file(k))
end

#将boot.o调到最前面
if OBJECTS.include?("boot.o")
    OBJECTS.delete("boot.o")
    OBJECTS.unshift("boot.o")
end

MAKEFILE = File.open("Makefile", "w")
MAKEFILE.print("Taurix : #{OBJECTS.join(" ")}\n\t#{LINK} #{OBJECTS.join(" ")} -o Taurix -Ttext=#{SETTINGS["Architecture"]["x86_64"]["base address"]} --entry=start #{LINK_FLAGS_COMMON}\n")

SOURCES.each do |src, dep|
    obj = src_obj_file(src)
    MAKEFILE.print("#{obj} : #{src} #{dep.join(" ")}\n\t")
    if C_EXTNAMES.include?(File.extname(src)) 
        MAKEFILE.print("#{CC} -c #{src} -o #{obj} #{CC_FLAGS_COMMON}\n")
    elsif CXX_EXTNAMES.include?(File.extname(src))
        MAKEFILE.print("#{CXX} -c #{src} -o #{obj} #{CXX_FLAGS_COMMON}\n")
    elsif ASM_EXTNAMES.include?(File.extname(src))
        MAKEFILE.print("#{ASM} #{src} -o #{obj} #{ASM_FLAGS_COMMON}\n")
    end
end

#制成由grub引导的硬盘镜像文件的规则
MAKEFILE.print("Taurix.img: Taurix\n\t")
MAKEFILE.print("cp Taurix ./TaurixFS/boot\n\t")
MAKEFILE.print("grub-mkrescue -o Taurix.img TaurixFS\n")

#别名 Image <-> Taurix.img
MAKEFILE.print("Image:\n\tmake Taurix.img\n")

#清除规则
MAKEFILE.print("clean:\n\t")
MAKEFILE.print("#{RM} *.o\n\t")
MAKEFILE.print("#{RM} Taurix\n\t")
MAKEFILE.print("#{RM} Taurix.img\n\t")
MAKEFILE.print("#{RM} ./TaurixFS/boot/Taurix\n")

MAKEFILE.close

#生成使用qemu执行的脚本
QEMU_SCRIPT =<<AAAA
#!/usr/bin/sh
make Image
if [ $? -eq 0 ]; then
    env LANG=en_US.UTF8 qemu-system-#{$arch} Taurix.img %s
fi    
AAAA

QEMURUN = File.open("QemuRun.sh", "w") do |f|
    f.print(sprintf(QEMU_SCRIPT, ""))
end
system("chmod +x QemuRun.sh")

QEMUDEBUG = File.open("QemuDebug.sh", "w") do |f|
    f.print(sprintf(QEMU_SCRIPT, "-S -s"))
end
system("chmod +x QemuDebug.sh")