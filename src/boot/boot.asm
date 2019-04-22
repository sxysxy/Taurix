[bits 16]

org 0x7c00

;清屏
mov ah, 0x06
mov bh, 0x07
mov al, 0
mov cx, 0
mov dx, 0xffff
mov bh, 0
int 0x10
;-------

;显示提示信息
mov ax, 0
mov es, ax
mov ax, _init_msg
mov bp, ax
mov ah, 0x13
mov al, 1
mov bh, 0
mov bl, 1  ;黑底蓝色字
mov cx, 15
mov dx, 0
int 0x10

;加载后续的程序，进行初始化


_loop2:
hlt
jmp _loop2

_init_msg: db 'Initializing...', 0

times 510-($-$$) db 0  ;补0
dw 0xaa55              ;引导扇区标记