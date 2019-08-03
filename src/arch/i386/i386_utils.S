; File: src/arch/i386/i386_utils.S
;   i386平台专用 汇编语言函数，实现include/taurix/arch/i386/i386_utils.h中声明的平台相关的接口
;      author: hfcloud(sxysxygm@gmail.com)
;        date: 2019.07.30

[bits 32]
global i386_set_gdtr, i386_set_idtr, i386_set_ldtr
global i386_get_cr0, i386_set_cr0
global i386_get_cr2, i386_set_cr2
global i386_get_cr3, i386_set_cr3
global i386_get_cr4, i386_set_cr4
global i386_ltr, i386_jmp_sel

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

;void i386_set_ldtr(uint16 selector)
i386_set_ldtr:
    mov eax, [esp+4]
    shl eax, 3
    mov [esp+4], eax
    lldt [esp+4]
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

;void i386_ltr(uint16 selector)
i386_ltr:
    mov eax, [esp+4]
    shl eax, 3
    mov [esp+4], eax 
    ltr [esp+4]
    ret

;提供一个C语言函数，能够实现 jmp 段选择子:偏移
;void i386_jmp_sel(uint16 selector, uint32 offset)
i386_jmp_sel:
    push ebp 
    mov ebp, esp

    mov eax, [ebp+8]
    shl ax, 3
    push eax 
    mov eax, [ebp+12]
    push eax 
    retf    ;模拟远调用返回
    
    leave
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