argv = ARGV.clone

param = {}
param[:files] = []

help = <<EOS
    -f file1, file2, ...           Set input binary files
    -s size                        Set output image file size in bytes(default as 1474560(1.44MB))
    -o filename                    Set output image file name
    -d directory                   Set the working directory(default as directory of ImageMaker.rb (File.dirname(__FILE__)) )
    -h                             Show this help
EOS

while arg = argv.shift
    case arg
    when /\A-f\z/    
        param[:files].push(argv.shift) while argv[0] && argv[0][0] != '-'
    when /\A-s\z/
        param[:size] = argv.shift.to_i
        if !param[:size] || param[:size] <= 0 
            puts "Invalid File size #{param[:size]}"
            exit(0)
        end
    when /\A-o\z/
        param[:output] = argv.shift
    when /\A-h\z/
        puts(help)
        exit(0)
    when /\A-d\z/
        param[:dir] = argv.shift
    else 
        puts(help)
        exit(0)
    end
end

param[:dir] = File.dirname(__FILE__) if !param[:dir]

if param[:files].size == 0
    puts("No input files, stop")
    exit(1)
end

param[:size] = 1474560 if !param[:size]

if !param[:output] 
    puts("No output file specified, stop")
    exit 1
end

actual_size = 0

File.open(File.join(param[:dir], param[:output]), "wb") do |of|
    param[:files].each do |inf|
        data = File.binread(File.join(param[:dir], inf))
        actual_size += data.size
        of.write(data)
    end
    if actual_size > param[:size] 
        puts("Warning: output file size exceeded, limit:#{param[:size]}, actual:#{actual_size}")
    else
        (param[:size] - actual_size).times do
            of.write("\0")
        end
    end
end


