#encoding: utf-8
top_dir = File.dirname(__FILE__)            #当前目录作为顶级目录
rake_dirs = ["./src/boot"]

bochs_dir = File.join(top_dir, "toolchain/Bochs-2.6.9")
bochsrc = File.join(top_dir, "img/bochsrc.bxrc")

def wsl(command)
    $is_windows ||= ((RUBY_PLATFORM =~ /win/) || (RUBY_PLATFORM =~ /mingw/))
    if $is_windows
        sh "wsl #{command}"
    else 
        sh command
    end
end

task :build do |t| 
    rake_dirs.each do |dir|
        sh "cd #{dir}&rake build"
    end
    Rake::Task['img/TaurixOS.img'].invoke
end

task :clean do |t|
    rake_dirs.each do |dir|
        sh "cd #{dir}&rake clean"
    end
    wsl "rm -r ./img" rescue nil
end

BOCHSRC_TEXT = <<EOS
#内存大小
megs: 256

#ROM镜像
romimage: file=#{bochs_dir}/BIOS-bochs-latest
vgaromimage: file=#{bochs_dir}/VGABIOS-lgpl-latest

#设置软盘镜像
floppya: 1_44=#{File.join(top_dir, "img/TaurixOS.img")}, status=inserted

#从软盘启动
boot: a

#日志文件
log: ./img/bochslog.log

#启用鼠标
mouse: enabled=0

#键位设置
keyboard: keymap=#{bochs_dir}/keymaps/x11-pc-us.map
EOS

file "img/bochsrc.bxrc" do |t|
    File.open("img/bochsrc.bxrc", "w") do |f| 
        f.print(BOCHSRC_TEXT)
    end
end

task :run do |t|
    Rake::Task[:build].invoke
    Rake::Task["img/bochsrc.bxrc"].invoke
    sh "#{bochs_dir}/bochs -f #{File.join(top_dir, "img/bochsrc.bxrc")}"
end

task :debug do |t|
    Rake::Task[:build].invoke
    Rake::Task["img/bochsrc.bxrc"].invoke
    sh "#{bochs_dir}/bochsdbg -f #{File.join(top_dir, "img/bochsrc.bxrc")}"
end

file 'img/TaurixOS.img' => ["src/boot/boot.bin", "src/boot/init.bin"] do |t|
    sh "mkdir img" if !File.exist?("./img")    
    sh "ruby ./misc/ImageMaker.rb -d #{top_dir} -f #{t.prerequisites.join(' ')} -o #{t.name}"
end