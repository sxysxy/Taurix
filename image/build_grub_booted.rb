#!/bin/ruby
#encoding: utf-8

require 'fileutils'
Dir.chdir(File.dirname(__FILE__))

if !File.exist?("template.img")
        
    if RUBY_PLATFORM =~ /mingw/ || RUBY_PLATFORM =~ /mswin/
        FileUtils.cp("../toolchain/template.img", "Taurix.img")
    else 
        puts("template.img not found, now download it.")
        system("wget https://github.com/sxysxy/Taurix/releases/download/v0.11/template.img")
        FileUtils.cp("template.img", "Taurix.img")
    end
   
end

if !system("tfstools Taurix.tfs > ImageBuild.log")
    FileUtils.rm("Taurix.img")
    puts("Failed to build Taurix.img")
    exit 1
end

    
