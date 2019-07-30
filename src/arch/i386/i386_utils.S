; File: src/arch/i386/i386_utils.S
;   i386平台专用 汇编语言函数，实现include/taurix/arch/i386/i386_utils.h中声明的平台相关的接口
;      author: hfcloud(sxysxygm@gmail.com)
;        date: 2019.07.30

[bits 32]
global i386_set_gdtr, i386_set_idtr
global i386_get_cr0, i386_set_cr0
global i386_get_cr2, i386_set_cr2
global i386_get_cr3, i386_set_cr3
global i386_get_cr4, i386_set_cr4

global reload_selectors ;仅在src/arch/i386/arch_init.c内部使用 

;void i386_set_gdtr(uint16 limit, void *addr)
i386_set_gdtr:
    mov ax, [esp+4]
    mov [esp+6], ax
    lgdt [esp+6]
    ret

;void i386_set_idtr(uint16 limit, void *addr)
i386_set_idtr:
    mov ax, [esp+4]
    mov [esp+6], ax
    lidt [esp+6]
    ret

i386_get_cr0:
    mov eax, cr0
    ret

i386_set_cr0:
    mov eax, [esp+4]
    mov cr0, eax
    ret

i386_get_cr2:
    mov eax, cr2
    ret

i386_set_cr2:
    mov eax, [esp+4]
    mov cr2, eax
    ret

i386_get_cr3:
    mov eax, cr3
    ret

i386_set_cr3:
    mov eax, [esp+4]
    mov cr3, eax
    ret

i386_get_cr4:
    mov eax, cr4
    ret

i386_set_cr4:
    mov eax, [esp+4]
    mov cr4, eax
    ret

reload_selectors:
SELECTOR_INDEX_CODE32_KERNEL equ 8
SELECTOR_INDEX_DATA32_KERNEL equ 16
    mov ax, SELECTOR_INDEX_DATA32_KERNEL
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    jmp SELECTOR_INDEX_CODE32_KERNEL:reload_selectors_ok
reload_selectors_ok: 
    ret