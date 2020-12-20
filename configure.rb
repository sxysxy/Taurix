#!env ruby
#encoding: utf-8

require 'find'

DIR_ROOT = Dir.pwd
SRC_DIR = "./src"
def is_windows 
    $_is_windows ||= RUBY_PLATFORM =~ /mingw/ || RUBY_PLATFORM =~ /mswin/ || RUBY_PLATFORM =~ /msys/
    $_is_windows
end
def is_osx
    $_is_osx ||= RUBY_PLATFORM =~ /darwin/
    $_is_osx
end
def is_linux
    $_is_linux ||= RUBY_PLATFORM =~ /linux/
    $_is_linux
end
def mkdir(dir)
    if dir && (!Dir.exist?(dir))
        begin 
            Dir.mkdir(dir)
        rescue
            return false
        end
    end
    return true
end

class Array 
    if !self.instance_methods.include?(:filter)
        alias :filter :select
    end
end

class ConfigHolder
    include Enumerable

    def initialize(name = "")
        @config = {}
        @name = name
    end
    def __name__
        @name 
    end
    def method_missing(name, *arg, &block)
        @config[name] ||= []
        if block == nil
            if arg.size == 0
                return @config[name]
            else 
                @config[name].push(arg)
            end
        else 
            return @config[name] if arg.size == 0
            x = ConfigHolder.new(arg[0]) 
            x.instance_exec(&block)
            @config[name].push(x) 
        end
    end
end

class ConfigLoader
    def initialize
        @config = {}
    end
    define_method(:config) do |name, &block|
        if @config[name.to_sym]
            raise ArgumentError, "Config Section #{name} has already been in existance"
        end
        x = ConfigHolder.new(name.to_s)
        x.instance_exec(&block)
        @config[name.to_sym] = x
    end
    def [](name) 
        if @config[name.to_sym] 
            return @config[name.to_sym]
        else
            #raise ArgumentError, "#{name} config section not found"
            return (@config[name.to_sym] = ConfigHolder.new(name.to_s))
        end
    end

    def self.load(filename)
        x = self.new
        x.instance_eval(File.read(filename))
        return x
    end
end

CONFIG_ARGV = ARGV.clone

bits32 = false
configf = (ARGV && ARGV[0]) ? ARGV[0] : "./configuration.rb"

if !File.exist?(configf) 
    configf = "./configuration.rb"
    if !File.exist?(configf)
        puts "No configuration file specified, abort"
        exit 1
    end
else  
    CONFIG_ARGV.delete(configf)
end

bits32 = (ARGV && ARGV.include?("--bits32"))
static = (ARGV && ARGV.include?("--static"))
debug = (ARGV && ARGV.include?("--debug"))
CONFIG_ARGV.delete("--bits32")
CONFIG_ARGV.delete("--debug")
CONFIG_ARGV.delete("--static")

if(ARGV && ARGV.include?("--help"))
    print(%{
        --static : For release build
        --bits32 : For 32-bits build(There should be 32-bits C++ toolchain in the environment)
        --debug : Disable code optimization and enable code debug.
    })
    exit 0
end

define_method(:argv) { |*arg| 
    CONFIG_ARGV
}

define_method(:is_bits32) {|*arg| 
    bits32
} 

define_method(:is_release) {|*arg|
    !debug
}

define_method(:is_debug) {|*arg|
    debug
}

define_method(:set_pure_c_language) {|c|
    if c 
        $pure_c = true
    end
}

#Read config
Config = ConfigLoader.load(configf)

TOOL_CC = "gcc"
TOOL_CC_C = "gcc -c"
TOOL_CXX = "g++"
TOOL_CXX_C = "g++ -c"
ASM = "nasm"
RM = "rm -f"

if $pure_c
    CXX = TOOL_CXX
    CXX_C = TOOL_CXX_C 
    CC = TOOL_CC
    CC_C = TOOL_CC_C
else 
    CXX = TOOL_CXX
    CXX_C = TOOL_CXX_C
    CC = TOOL_CXX
    CC_C = TOOL_CXX_C
end

begin 
    $build_dir = Config["Outputs"].build_dir.flatten[0]
    $exec_dir = Config["Outputs"].exec_dir.flatten[0] 

    if !mkdir($build_dir)
        print("Failed to create build_dir #{$build_dir}, please check your permission\n");
        exit(1)
    end

    if !mkdir($exec_dir)
        print("Failed to create exec_dir #{$exec_dir}, please check your permission\n");
        exit(1)
    end
rescue
end

#for show
_CXX_FLAGS_COMMON = [Config["Options"].flag.flatten.join(" "), is_debug ? "-O0 -g" : "-O2"] + 
                        Config["PreDefine"].define.map {|d| "-D#{d[0]}=#{d[1]}"}
_CXX_FLAGS_COMMON.push("--static") if static
if is_windows
    if !is_bits32
        _CXX_FLAGS_COMMON.push("-DWIN64=1")
        _CXX_FLAGS_COMMON.push("-D_WIN64=1")
    end 
    _CXX_FLAGS_COMMON.push("-DWIN32=1")
    _CXX_FLAGS_COMMON.push("-D_WIN32=1")
end
#Config["ThirdParties"].link_dir.push(".") #加入当前路径
Config["ThirdParties"].link_dir.push($exec_dir) if $exec_dir #加入exec_dir
_CXX_LINKS = Config["ThirdParties"].link_dir.flatten.map{|d| "-L#{d}"} + Config["ThirdParties"].link.flatten.map{|l| "-l#{l}"}
_CXX_INCLUDE_DIRCTORIES = Config["ThirdParties"].include.flatten.map {|i| "-I#{i}"}
CXX_FLAGS_COMMON = _CXX_FLAGS_COMMON.join(' ') + " " + _CXX_INCLUDE_DIRCTORIES.join(' ') + " " + _CXX_LINKS.join(' ') 

DIR_SRC = File.join(DIR_ROOT, ".")
SRC_EXTNAMES = [".c", ".cpp", ".cc", ".cxx"]

def entry_name(file)
    n = (is_windows ? (file + ".exe") : file)
    return ($exec_dir ?  File.join($exec_dir, n) : n) 
end

def shared_name(file)
    n = file
    n = "lib" + file + ".dll" if is_windows
    n = "lib" + file + ".so" if is_linux
    n = file + ".dylib" if is_osx
    return ($exec_dir ?  File.join($exec_dir, n) : n) 
end

class CustomBuildInput 
    attr_accessor :intermedia, :depends
end

class CustomBuildOutput
    attr_accessor :target_name, :depends, :commands

    def initialize
        @target_name = ""
        @depends = []
        @commands = []
    end

end 

class Generator
    attr_reader :sources, :objects
    attr_reader :entries, :shareds

    def initialize
        @sources = {}
        @asm = []
        @objects = []

        @entries = {}
        @shareds = {}

        @@cache ||= {}
    end

    def scan_file_dependence(file, src = nil)
        ext = File.extname(file)
        dir = File.dirname(file)

        return false if (!SRC_EXTNAMES.include?(ext) && !src)
        is_src = (src == nil)
        src = file if !src

        if @@cache[src]
            @sources[src] = @@cache[src]
            return true 
        end

        fp = File.open(file)
        @sources[src] ||= []

        fp.each_line do |line|
            #code = line.force_encoding("utf-8")
            #p line.encoding
            code = line.unpack("C*").pack("U*")
            /#include\s*["<](.+)[">]/.match(code)
            if $1
                ok = false
                dep = File.join(dir, $1)
                if File.exist?(dep)
                    @sources[src].push(dep)
                    ok = true 
                else 
                    dep = File.join(SRC_DIR, $1)
                    if File.exist?(dep)
                        @sources[src].push(dep)
                        ok = true
                    end
                end
                if ok 
                    scan_file_dependence(dep, src)
                end
            end
        
        end
        fp.close
        @@cache[src] = @sources[src]
        return true
    end

    def src_obj_file(file)
        @@src_obj_cache ||= {}
        #return @@src_obj_cache[file] if @@src_obj_cache[file]

        if !(@@src_obj_cache[file])
            sp = File.split(file)
            base = sp[1]
            i = base.length-1
            i -= 1 while base[i] != "."
            @@src_obj_cache[file] = sp[0].gsub!(/[\/\.\\]/, "_") + "_" + base[0, i] + ".o"  
        end
        n = @@src_obj_cache[file]
        return ($build_dir ?  File.join($build_dir, n) : n) 
    end

    def src_obj_file_with_module(file, module_name)
        src_obj_file(file)
        n = "_#{module_name}_#{@@src_obj_cache[file]}"
        return ($build_dir ?  File.join($build_dir, n) : n) 
    end

    def entry_exe_file(file)
        entry_name(file)
    end

    def shared_file(file)
        shared_name(file)
    end

    def compile_src(src) 
        obj = src_obj_file(src)
        "#{CXX_C} #{src} -o #{obj} #{CXX_FLAGS_COMMON}\n\t"
    end

    def pattern_to_files(pattern) 
        sp = File.split(pattern)
        files = Find.find(sp[0]).to_a
        files = files.filter {|f| (File.file? f)}
        begin 
            @@pattern_cache ||= {}
            @@pattern_cache[sp[1]] ||= Regexp.new(sp[1].gsub(".", "\\.").gsub("*", "\\S"))
            pt = @@pattern_cache[sp[1]]
            return (files.filter {|f| (f =~ pt) != nil })
        rescue 
            return files
        end
    end 

    class SpecificDefines
        attr_reader :files, :defines, :flags 
        def initialize(_files, _defines, _flags)
            @files = _files
            @defines = _defines  
            @flags = _flags
        end
    end

    def generate(filename) 
        @@specfic_defines = {}
        @@depends = {}
        @@after = {}
        @@customs = {}
        @@renames = {}

        puts "Entries:"
        @@rec = @entries

        enum_entry_shared = lambda do |config|
            final_name = nil
            if @@rec.equal? @entries
                final_name = entry_exe_file(config.__name__)
            else
                final_name = shared_file(config.__name__) 
            end
            puts "    #{final_name}"

            @@rec[config.__name__] ||= []
            have_specifc_defines = (config.define.size > 0 || config.flag.size > 0)
            @@specfic_defines[config.__name__] = nil
            all_files = []
            config.src.flatten.each {|pattern| 
                files = pattern_to_files(pattern)    
                files.each {|src|
                    @@rec[config.__name__].push(have_specifc_defines ? src_obj_file_with_module(src, config.__name__) : src_obj_file(src)) if scan_file_dependence(src)
                } 
                all_files += files
                if files.size == 1 
                    if !File.exist?(files[0])
                        puts "Source file #{files[0]} specified but it does not exist"
                        exit 1
                    end
                end
            }

            config.asm.flatten.each {|pattern|
                files = pattern_to_files(pattern)
                files.each {|src|
                    @@rec[config.__name__].push(src_obj_file(src))
                }
                #all_files += files
                @asm += files
            }
            
            if have_specifc_defines
                @@specfic_defines[final_name] = SpecificDefines.new(all_files, config.define, config.flag)
            end
            if config.depends && config.depends.size > 0 
                #@@depends[final_name] = "#{}"
                @@depends[final_name] = config.depends.flatten
            else 
                @@depends[final_name] = []
            end

            if config.after && config.after.size > 0
                @@after[final_name] = config.after.flatten 
            end

            if config.rename 
                @@renames[final_name] = config.rename
            end

            if config.custom 
                @@customs[final_name] = config.custom
            end
        end

        Config["Executable"].entry.each(&enum_entry_shared)

        puts "Shared libraries:"
        @@rec = @shareds
        Config["Executable"].shared.each(&enum_entry_shared)

        @sources.each_key do |k|
            next if @entries.include? k 
            @objects.push(src_obj_file(k))
        end

        makefile = File.open(filename, "w")

        @entries.each do |entry_name, relative_objects|
            entry = entry_exe_file(entry_name)
            _objects = (relative_objects.flatten.uniq).join(" ")
            deps_link = (@@depends[entry].flatten.map{|x|" -l#{x}"}).join('');
            #deps_link = ($exec_dir ? " -L#{$exec_dir}/" : "") + (@@depends[entry].flatten.map{|x|" -l#{x}"}).join('')
            #after:
            after = (@@after[entry] && @@after[entry].size > 0) ? @@after[entry] : []
            after_final = []
            after.each {|a| 
                if @entries[a]
                    after_final << entry_name(a)
                end
                if @shareds[a]
                    after_final << shared_name(a)
                end
            }
                
            exec_custom = lambda {|obj|
                ci = CustomBuildInput.new 
                ci.intermedia = obj 
                ci.depends = @@depends[entry]
                co = instance_exec { @@customs[entry].flatten[0].call(ci) }
                if !co.is_a? CustomBuildOutput
                    raise "Return value of a custom should be a CustomBuildOutput"
                end
                makefile.print("#{co.target_name.to_s}: #{co.depends.join(' ')}\n")
                makefile.print("\t#{co.commands.join("\n\t")}\n")
            }

            if @@specfic_defines[entry] 
                pd = @@specfic_defines[entry]
                flags = pd.flags.flatten.join(' ')
                defines = pd.defines.map {|d| "-D#{d[0]}=#{d[1]}"}
                makefile.print("#{entry}: #{after_final.join(' ')} #{_objects} #{@@depends[entry].map{|x| shared_name(x)}.join(' ')}\n\t#{CXX} -o #{entry} #{_objects} #{flags} #{CXX_FLAGS_COMMON} -Wl,-Bdynamic#{deps_link}\n")
                specifc_objs = []
                pd.files.each do |src|
                    obj = src_obj_file_with_module(src, entry_name)
                    #puts "#{src} #{@sources[src] == nil}"
                    
                    specifc_objs << obj
                    makefile.print("#{obj} : #{src} #{@sources[src].join(" ")}\n\t")
                    makefile.print("#{CXX_C} #{src} -o #{obj} #{defines.join(" ")} #{flags} #{CXX_FLAGS_COMMON}\n")
                

                end
                exec_custom.call(specifc_objs)
            else    #normal
                if !@@customs[entry]
                    makefile.print("#{entry}: #{after_final.join(' ')} #{_objects} #{@@depends[entry].map{|x| shared_name(x)}.join(' ')}\n\t#{CXX} -o #{entry} #{_objects} #{CXX_FLAGS_COMMON} -Wl,-Bdynamic#{deps_link}\n")
                else
                    exec_custom.call(relative_objects.flatten.uniq)
                end
            end
        end
        
        @shareds.each do |shared_name, relative_objects|
            shared = shared_file(shared_name)
            _objects = (relative_objects.flatten.uniq).join(" ")
            deps_link = (@@depends[shared].flatten.map{|x|" -l#{x}"}).join('')
            #deps_link = ($exec_dir ? " -L#{$exec_dir}/" : "") + (@@depends[shared].flatten.map{|x|" -l#{x}"}).join('')
            after = (@@after[entry] && @@after[entry].size > 0) ? @@after[entry] : []
            after_final = []
            after.each {|a| 
                if @entries[a]
                    after_final << entry_name(a)
                end
                if @shareds[a]
                    after_final << shared_name(a)
                end
            }
             
            if @@specfic_defines[shared] 
                pd = @@specfic_defines[shared]
                defines = pd.defines.map {|d| "-D#{d[0]}=#{d[1]}"}
                flags = pd.flags.flatten.join(' ')
                makefile.print("#{shared}: #{after_final.join(' ')} #{_objects} #{@@depends[shared].map{|x| shared_name(x)}.join(' ')}\n\t#{CXX} -o #{shared} #{_objects} --shared #{flags} #{CXX_FLAGS_COMMON} -Wl,-Bdynamic#{deps_link}\n")
                pd.files.each do |src|
                    obj = src_obj_file_with_module(src, shared_name)
                    makefile.print("#{obj} : #{src} #{@sources[src].join(" ")}\n\t")
                    makefile.print("#{CXX_C} #{src} -o #{obj} #{defines.join(" ")} #{flags} #{CXX_FLAGS_COMMON}\n")
                end
            else  #normal
                makefile.print("#{shared}: #{after_final.join(' ')} #{_objects} #{@@depends[shared].map{|x| shared_name(x)}.join(' ')}\n\t#{CXX} -o #{shared} #{_objects} --shared #{CXX_FLAGS_COMMON} -Wl,-Bdynamic#{deps_link}\n")
            end
        end
        
        @sources.each do |src, dep| 
            obj = src_obj_file(src)

            compile = TOOL_CXX
            compile_c = TOOL_CXX_C
            if $pure_c 
                
                if File.extname(src) =~ /\.cpp\b|\.cc\n/i 
                    compile = TOOL_CXX 
                    compile_c = TOOL_CXX_C
                else 
                    compile = TOOL_CC 
                    compile_c = TOOL_CC_C
                end
            end
            
            makefile.print("#{obj} : #{src} #{dep.join(" ")}\n\t")
            makefile.print("#{compile_c} #{src} -o #{obj} #{CXX_FLAGS_COMMON}\n")
            
        end

        @asm.each do |a| 
            obj = src_obj_file(a)
            makefile.print("#{obj} : #{a}\n\t")
            makefile.print("#{ASM} -f elf #{a} -o #{obj}\n")
        end

        #make all
        #makefile.print("all:")
        targets = []
        @entries.each_key do |entry|
            #makefile.print("\n\t+make #{entry_exe_file(entry_name)}")
            targets << entry_name(entry)
        end
        @shareds.each_key do |shared|
            #makefile.print("\n\t+make #{shared_file(shared_name)}")
            targets << shared_name(shared)
        end
        makefile.print("all: #{targets.join(' ')}\n\techo Build OK\n\n")

        #make clean

        makefile.print("clean:\n\t#{RM} #{$build_dir ? File.join($build_dir, "*.o") : "*.o"}")
        @entries.each_key do |entry_name|
            entry_file = entry_exe_file(entry_name)
            makefile.print("\n\t#{RM} #{entry_file}")
        end
        @shareds.each_key do |shared_name|
            _shared_file = shared_file(shared_name)
            makefile.print("\n\t#{RM} #{_shared_file}")
        end
        makefile.close

    end
end


g = Generator.new
g.generate("Makefile")
