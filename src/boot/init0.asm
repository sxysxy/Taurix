; init0.asm
; 初始化程序，负责进入保护模式，调用C语言的init1函数
;        author: HfCloud(sxysxygm@gmail.com)

global init0_start  ;导出这个符号
extern _init1
INIT_BASE_ADDR equ 0x10000

[bits 16]
init0_start:

jmp _prepare
dw 0
dd 0  ;GDT表前补齐8字节

_GDT:  ;临时GDT表
;空描述符
dq 0

;段基地址0，限4G，限R0权限，可读可写
_GDT_DATA:
dw 0xffff  ;byte 0~1
dw 0       ;byte 2~3
db 0       ;byte 4
db 10010010b ;byte 5
db 11001111b ;byte 6
db 0         ;byte 7
DataSelector equ _GDT_DATA - _GDT 

;段基地址0，限4G，限R0权限，可执行
_GDT_CODE:
dw 0xffff  ;byte 0~1
dw 0       ;byte 2~3
db 0       ;byte 4
db 10011010b ;byte 5
db 11001111b ;byte 6
db 0         ;byte 7
CodeSelector equ _GDT_CODE - _GDT

;栈段，基地址0x50000, 段限0xf0000，限R0权限，可读可写
_GDT_STACK:
dw 0         ;byte 0~1
dw 0         ;byte 2~3
db 5         ;byte 4
db 10010110b ;byte 5
db 01001111b ;byte 6
db 0         ;byte 7
StackSelector equ _GDT_STACK - _GDT

dq 0

GdtLen equ $ - _GDT
_GDT_info:
dw GdtLen-1 
dd _GDT + INIT_BASE_ADDR

_prepare:
cli

;加载GDT
mov ax, cs
mov ds, ax
lgdt [_GDT_info]

;开A20地址线
in al, 0x92
or al, 00000010b
out 0x92, al

;置cr0寄存器PE位
mov eax, cr0
or eax, 1
mov cr0, eax

;跳转保护模式
jmp dword CodeSelector:(_protected + INIT_BASE_ADDR)

[bits 32]
_protected:
mov ax, DataSelector
mov ds, ax
;mov ax, StackSelector
mov ss, ax
mov ebp, 0x60000   ;栈顶指针

call _init1

_hlt:
hlt
jmp _hlt

