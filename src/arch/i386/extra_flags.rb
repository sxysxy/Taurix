_CC_FLAGS_COMMON.push("-m32 -Wno-int-to-pointer-cast -Wno-pointer-to-int-cast")
_CXX_FLAGS_COMMON.push("-m32 -Wno-int-to-pointer-cast -Wno-pointer-to-int-cast")
_ASM_FLAGS_COMMON.push("-f elf")

if RUBY_PLATFORM =~ /darwin/ #fuck OSX CLANG
    _LINK_FLAGS_COMMON.push("-arch i386")
else 
    _LINK_FLAGS_COMMON.push("-m elf_i386")
end
