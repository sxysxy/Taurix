; File: src/arch/i386 ints_wrapper.S
;        调用C语言编写的中断处理函数的包装
;            author: hfcloud(sxysxy@gmail.com)
;              date:  2019.08.03
;

[bits 32]

global simulate_iret
extern ru_memcpy

%macro set_handler 2
global %1
extern %2
%1:   
        ;保存中断之前的状态
    push gs
    push fs
    push es 
    push ds
    
    pushad  

    push eax 
    ; high <------------------------------------------------------------> low
    ; [ss0, esp0, eflags, cs, eip, gs, fs, es, ds, pushad(64bytes), eax(tmp)]
    ;                                                                   ↑       
    ;                                                                  esp  

    ;切换到内核栈和数据段进行中断处理
    xor eax, eax
    mov ax, ss 
    mov ds, ax 
    mov es, ax
    mov fs, ax
    mov gs, ax
    pop eax    ;恢复eax

    mov [esp-4], esp
    sub esp, 4   
    call %2  
    add esp, 4
    
    popad       ;balance pushad
    
    pop ds 
    pop es
    pop fs 
    pop gs
    iretd       ;从断点恢复执行，如果在中断处理程序中修改eip, cs, eflags, esp0, ss0，即可实现任务切换
%endmacro    

set_handler entry_exception_de_handler, exception_de_handler
set_handler entry_exception_df_handler, exception_df_handler
set_handler entry_exception_gp_handler, exception_gp_handler

set_handler entry_clock_int_handler, clock_int_handler  ;->!
set_handler entry_keyboard_int_handler, keyboard_int_handler

;模拟中断返回
;void simulate_iret(Context *context)
simulate_iret:
    
    mov eax, [esp+4]  ;eax <- src(context)

    ;传入memcpy的参数，因为push会破坏esp寄存器，所以使用mov指令代替push
    sub esp, 60
    mov dword [esp-4], 60
    mov [esp-8], eax 
    mov eax, esp
    mov [esp-12], eax 
    sub esp, 12      
    call ru_memcpy
    add esp, 12         ;3*4

                ;这时候esp恰好指向目标Context
    popad       ;模拟恢复现场 
    pop ds 
    pop es
    pop fs 
    pop gs
    iretd 

    ;should not reach here
    ret

;模拟中断返回，返回目标在低特权级
;void simulate_iret_to_user(Context *context)
simulate_iret_to_user:
    
    mov eax, [esp+4]  ;eax <- src(context)

    ;传入memcpy的参数，因为push会破坏esp寄存器，所以使用mov指令代替push
    sub esp, 68
    mov dword [esp-4], 68
    mov [esp-8], eax 
    mov eax, esp
    mov [esp-12], eax 
    sub esp, 12      
    call ru_memcpy
    add esp, 12         ;3*4

                ;这时候esp恰好指向目标Context
    popad       ;模拟恢复现场 
    pop ds 
    pop es
    pop fs 
    pop gs
    iretd 

    ;should not reach here
    ret



    
