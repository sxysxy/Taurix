#!/bin/ruby
#encoding: utf-8

require 'fileutils'
Dir.chdir(File.dirname(__FILE__))

puts("template.img not found, now download it.")
if !File.exist?("template.img")
    system("wget https://github.com/sxysxy/Taurix/releases/download/v0.11/template.img")
end

FileUtils.cp("template.img", "Taurix.img")
if !system("tfstools Taurix.tfs > ImageBuild.log")
    FileUtils.rm("Taurix.img")
    puts("Failed to build Taurix.img")
    exit 1
end

    
