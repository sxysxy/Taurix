[bits 64]

;exports
global ru_port_write8, ru_port_write16
global ru_port_read8, ru_port_read16
global ru_enable_interrupt, ru_disable_interrupt

ru_port_write8:
    push rdx
    mov rax, rdx  ;rax<-data
    mov rdx, rcx  ;rdx<-port
    out dx, al
    pop rdx
    ret

ru_port_write16:
    push rdx
    mov rax, rdx, 
    mov rdx, rcx
    out dx, ax
    pop rdx
    ret 

ru_port_read8:
    push rdx
    mov rdx, rcx  ;rdx<-port
    xor rax, rax
    in al, dx
    pop rdx
    ret

ru_port_read16:
    push rdx
    mov rdx, rcx  ;rdx<-port
    xor rax, rax
    in ax, dx
    pop rdx
    ret

ru_enable_interrupt:
    sti
    ret

ru_disable_interrupt:
    cli
    ret
