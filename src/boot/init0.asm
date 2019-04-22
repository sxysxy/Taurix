; init0.asm
; 初始化程序，负责进入保护模式，调用C语言的init1函数
;        author: HfCloud(sxysxygm@gmail.com)
ORG 0x10000

[bits 16]

jmp _prepare

_GDT:  ;临时GDT表
;空描述符
dq 0

;段基地址0，限4G，限R0权限，可读可写
_GDT_DATA
dw 0xffff  ;byte 0~1
dw 0       ;byte 2~3
db 0       ;byte 4
db 00010010b ;byte 5
db 11001111b ;byte 6
db 0         ;byte 7
DataSelector equ _GDT_DATA - _GDT 

;段基地址0，限4G，限R0权限，可读可执行
_GDT_CODE
dw 0xffff  ;byte 0~1
dw 0       ;byte 2~3
db 0       ;byte 4
db 00011010b ;byte 5
db 11001111b ;byte 6
db 0         ;byte 7
CodeSelector equ _GDT_CODE - _GDT

GdtLen equ $ - _GDT
_GDT_info:
dw GdtLen
dd _GDT

_prepare:
cli

;加载GDT
mov ax, 0
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
mov ax, DataSelector
mov ds, ax
jmp dword CodeSelector:_protected

_protected:

_hlt:
hlt
jmp _hlt

