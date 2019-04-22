; boot.asm
; 引导扇区代码，被编译后放到镜像文件的第一个扇区，并把后续代码加载到 INIT_ADDR处
;    author: HfCloud(sxysxygm@gmail.com)
[bits 16]

org 0x7c00

INIT_ADDR equ 0x10000     ;init程序加载到这里
INIT_ADDR_SEG equ 0x1000  ;INIT_ADDR / 16

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


;jmp _load
;加载后续的程序，进行初始化
;struct DiskAddressPacket {
;   BYTE PacketSize; //16
;   BYTE Reserved;   //0
;   WORD BlockCount;  //扇区数
;   DWORD BufferAddr; //内存缓冲区地址
;   QWWORD BlockNum;  //起始块地址(第一扇区为0号)
;};

;DAP:
;db 16
;db 0
;dw 500  ;读取1000个扇区的512kb的数据 
;dw 0
;dw INIT_ADDR_SEG
;dq 1

;_load:
;mov ax, 0
;mov ds, ax
;mov si, DAP
;mov dl, 0      ;软盘
;mov ah, 0x42
;int 0x13
;jc _read_err

mov ax, INIT_ADDR_SEG
mov es, ax
mov bx, 0
mov dl, 0 ;软盘
mov ah, 2 ;读取
mov ch, 0 ;柱面
mov dh, 0 ;磁头
mov cl, 2 ;扇区号
mov al, 70 ;扇区个数
int 0x13
jc _read_err

jmp word INIT_ADDR_SEG:0

_read_err:
mov ax, 0
mov es, ax
mov ax, _err_msg
mov bp, ax
mov ah, 0x13
mov al, 1
mov bh, 0
mov bl, 1  ;黑底蓝色字
mov cx, 10
mov dx, 0
int 0x10

;这里不应该被执行到！！！
_loop2:
hlt
jmp _loop2

_init_msg: db 'Initializing...'
_err_msg: db 'Load Error'


times 510-($-$$) db 0  ;补0
dw 0xaa55              ;引导扇区标记