[bits 32]

;exports
;global ru_port_write8, ru_port_write16
;global ru_port_read8, ru_port_read16
global ru_enable_interrupt, ru_disable_interrupt

ru_enable_interrupt:
    sti
    ret

ru_disable_interrupt:
    cli
    ret
