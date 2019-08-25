[bits 32]
[section .text]
global start, _start, main, _main
extern TaurixCMain
global g_multiboot_info_addr  ;导出multiboot信息地址

main:
_main:
start:
_start:
jmp multiboot_entry
align 4
dd 0x1badb002
dd 0x00000007 ;要求页对其，传递内存信息，传递图形信息
dd -(0x1badb002 + 0x00000007)
dd 0
dd 0
dd 0
dd 0
dd 0
dd 1
dd 80
dd 24
dd 0
multiboot_entry:

;记录multiboot_info数据的地址
mov eax, g_multiboot_info_addr
mov [eax], ebx

;初始化栈
STACK_SIZE equ 0x5000
mov esp, stack_area+STACK_SIZE

call TaurixCMain

fin:
hlt
jmp fin

g_multiboot_info_addr:
dd 0

;栈空间
stack_area: 
times STACK_SIZE db 0
