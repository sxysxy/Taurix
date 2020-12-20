FS = "./src/fs/*.c"
KERNEL = "./src/kernel/*.c"
MM = "./src/mm/*.c"
UTILS = "./src/utils/*.c"
IPC_CROSSPLATFORM = "./src/ipc.c"
set_pure_c_language(true)

require 'json'
require 'fileutils'

SETTINGS_FILENAME = "taurix-settings-final.json"
DIR_ROOT = Dir.pwd

#加载设置
->{STDERR.puts("#{SETTINGS_FILENAME} not found. Please run taurix-settings-gui.py to generate it."); exit 1}[] unless File.exist?(SETTINGS_FILENAME)
SETTINGS = JSON.parse(File.read(SETTINGS_FILENAME))
AVALIABLE_ARCH = SETTINGS["Architecture"].keys

if !File.exist?(SETTINGS_FILENAME)
    puts("#{SETTINGS_FILENAME} not found, run settings first")
    if !system("python taurix-settings-gui.py")
        puts("Failed to run settings, using default parameters");
        FileUtils.cp("taurix-settings.json", SETTINGS_FILENAME)
    end
end

$target_arch = nil 
$argv = argv()
puts($argv)

while $argv.length > 0
    op = $argv.shift 
    if op == "--arch"
        $target_arch = $argv.shift 
    end 
end

$target_arch ||= "i386"
puts("Config target architecture = #{$target_arch.to_s}")

BASE_ADDR = SETTINGS["Architecture"][$target_arch]["base address"] || 0x10000

config("Executable") {
    entry("Taurix") {
        src FS, KERNEL, MM, UTILS, IPC_CROSSPLATFORM, "./src/arch/#{$target_arch}/*.c"
        asm "./src/arch/#{$target_arch}/*.S"

        custom(lambda { |ci|
            objs = ci.intermedia
            
            boot_obj = ""
            objs.each {|obj|
                if obj.split("/").last =~ /boot\.o/
                    boot_obj = obj 
                end
            }
            objs.delete(boot_obj)
            objs.unshift(boot_obj)
            
            co = CustomBuildOutput.new
            co.target_name = "image/Taurix.img"
            co.depends = objs
            co.commands << "ld -m #{is_windows ? "i386pe" : "elf-i386"} #{objs.join(' ')} -o #{entry_name("Taurix")} -Ttext=#{BASE_ADDR} --entry=start"
            co.commands << "objcopy -O elf32-i386 #{entry_name("Taurix")} Taurix"
            co.commands << "ruby ./image/build_grub_booted.rb"

            return co
        })
    }
}

#第三方库的配置
config("ThirdParties") {
    #include路径
    include "./include"
}

#编译器选项
config("Options") {
    #flag "-std=c++11"

    flag "-fno-builtin", "-fno-stack-protector"

    if $target_arch == "i386"
        flag "-m32"
    end

    if is_windows 
        flag "-fno-leading-underscore", "-Wno-int-to-pointer-cast", "-Wno-pointer-to-int-cast"
    end
}

#预定义宏的设置
config("PreDefine") {
   
}

#编译输出文件路径的设置
config("Outputs") {
    if is_debug 
        build_dir "./build_debug"
        exec_dir "./bin_debug"
    elsif is_release
        build_dir "./build_release"
        exec_dir "./bin_release"
    end
}


#生成使用qemu执行的脚本
QEMU_SCRIPT =<<AAAA
#!/bin/ruby
#encoding: utf-8
if system("make")
    system("#{ (is_windows ? "toolchain\\\\qemu\\\\" : "") + "qemu-system-#{$target_arch}" + (is_windows ? ".exe" : "")} image/Taurix.img %s")
end
AAAA

File.open("QemuRun.rb", "w") do |f|
    f.print(sprintf(QEMU_SCRIPT, ""))
end
system("chmod +x QemuRun.rb") if !is_windows    

File.open("QemuDebug.rb", "w") do |f|
    f.print(sprintf(QEMU_SCRIPT, "-S -s"))
end  
system("chmod +x QemuDebug.rb") if !is_windows
puts("Makefile Generated")