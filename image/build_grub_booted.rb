#!/bin/ruby
#encoding: utf-8

require 'fileutils'
Dir.chdir(File.dirname(__FILE__))

FileUtils.cp("template.img", "Taurix.img")
if !system("tfstools Taurix.tfs > ImageBuild.log")
    FileUtils.rm("Taurix.img")
    puts("Failed to build Taurix.img")
    exit 1
end

    
