[bits 32]
[section .text]
global start, _start
extern TaurixCMain

start:
_start:
jmp multiboot_entry
align 4
dd 0x1badb002
dd 0x00000005
dd -(0x1badb002 + 0x00000005)
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

;初始化栈
STACK_SIZE equ 0x2000
mov esp, stack_area+STACK_SIZE

call TaurixCMain

fin:
hlt
jmp fin

;栈空间
stack_area: 
times STACK_SIZE db 0
