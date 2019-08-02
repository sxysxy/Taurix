; File: src/arch/i386 ints_wrapper.S
;        调用C语言编写的中断处理函数的包装
;            author: hfcloud(sxysxy@gmail.com)
;              date:  2019.08.03
;

[bits 32]

%macro set_handler 2
global %1
extern %2
%1:
    push es 
    push ds
    pushad 
    mov eax, esp 
    push eax 
    mov ax, ss 
    mov ds, ax 
    mov es, ax 
    call %2 
    pop eax 
    popad 
    pop ds 
    pop es
    iretd
%endmacro

set_handler entry_exception_de_handler, exception_de_handler
set_handler entry_exception_df_handler, exception_df_handler
set_handler entry_exception_gp_handler, exception_gp_handler

set_handler entry_keyboard_int_handler, keyboard_int_handler