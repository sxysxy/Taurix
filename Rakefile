#encoding: utf-8
top_dir = File.dirname(__FILE__)            #当前目录作为顶级目录
rake_dirs = ["./src/boot"]

bochs_dir = File.join(top_dir, "toolchain/Bochs-2.6.9")

img_TaurixSetup = File.join(top_dir, "img/TaurixSetup.img")
img_TaurixOS    = File.join(top_dir, "img/TaruixOS.img")
bochsrc = File.join(top_dir, "img/bochsrc.bxrc")

task :build do |t| 
    rake_dirs.each do |dir|
        system "cd #{dir}&rake build"
    end
    Rake::Task['img/TaurixSetup.img'].invoke
    #Rake::Task['TaurixOS.img'].invoke
end

task :clean do |t|
    rake_dirs.each do |dir|
        system "cd #{dir}&rake clean"
    end
    system "del .\\bin\\*.img"
end

BOCHSRC_TEXT = <<EOS
#内存大小
megs: 256

#ROM镜像
romimage: file=#{bochs_dir}/BIOS-bochs-latest
vgaromimage: file=#{bochs_dir}/VGABIOS-lgpl-latest

#设置软盘镜像
floppya: 1_44=#{img_TaurixSetup}, status=inserted

#从软盘启动
boot: a

#日志文件
log: /img/bochslog.log

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

file 'img/TaurixSetup.img' => ["src/boot/boot.bin", "src/boot/init0.bin"] do |t|
    sh "ruby misc/ImageMaker.rb -d #{top_dir} -f #{t.prerequisites.join(' ')} -o #{t.name}"
end