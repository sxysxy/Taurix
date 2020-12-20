; File: src/arch/i386/arch_utils.S
;   对utils.h中平台无关的接口的平台相关的实现
;   实现的接口见global导出的符号
;      author: hfcloud(sxysxygm@gmail.com)
;        date: 2019.07.30

[bits 32]

;exports
global ru_port_write8, ru_port_write16, ru_port_write32
global ru_port_read8, ru_port_read16, ru_port_read32
global ru_enable_interrupt, ru_disable_interrupt
global ru_kernel_suspend  ;挂起，系统停机

;void __cedecl ru_port_write8(uint32 port, uint8 data);
ru_port_write8:
    push ebp
    mov ebp, esp
    push edx
    mov edx, [ebp+8]  ;edx<-port
    mov eax, [ebp+12] ;eax<-data
    out dx, al
    pop edx
    xor eax, eax
    leave
    ret

;void __cdecl ru_port_write16(uint32 port, uint16 data)
ru_port_write16:
    push ebp
    mov ebp, esp
    push edx
    mov edx, [ebp+8]  ;edx<-port
    mov eax, [ebp+12] ;eax<-data
    out dx, ax
    pop edx
    xor eax, eax
    leave
    ret


;void __cdecl ru_port_write32(uint32 port, uint32 data)
ru_port_write32:
    push ebp
    mov ebp, esp
    push edx
    mov edx, [ebp+8]  ;edx<-port
    mov eax, [ebp+12] ;eax<-data
    out dx, eax
    pop edx
    xor eax, eax
    leave
    ret

;uint8 ru_port_read8(uint32 port)
ru_port_read8:
    push ebp
    mov ebp, esp
    push edx
    mov edx, [ebp+8]
    xor eax, eax
    in al, dx
    pop edx
    leave
    ret

;uint16 ru_port_read16(uint32 port)
ru_port_read16:
    push ebp
    mov ebp, esp
    push edx
    mov edx, [ebp+8]
    xor eax, eax
    in ax, dx
    pop edx
    leave
    ret

;uint32 ru_port_read32(uint32 port)
ru_port_read32:
    push ebp
    mov ebp, esp
    push edx
    mov edx, [ebp+8]
    xor eax, eax
    in eax, dx
    pop edx
    leave
    ret


ru_enable_interrupt:
    sti
    ret

ru_disable_interrupt:
    cli
    ret

ru_kernel_suspend:
    ru_suspend_l:
    hlt
    jmp ru_suspend_l
    ret