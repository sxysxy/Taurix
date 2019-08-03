/* File: src/arch/i386/pic.c
    init_pic的实现
         author: hfcloud(sxysxygm@gmail.com)
           date: 2019.07.30
 */

#include <taurix/utils.h>
#include <taurix/arch/i386/i386_utils.h>

int32 init_pic() {
    ru_port_write8(I8259_M_CTL_PORT, 0x11);
    i386_nop3();

    ru_port_write8(I8259_S_CTL_PORT, 0x11);
    i386_nop3();

    ru_port_write8(I8259_M_DATA_PORT, GATE_INDEX_IRQ0); 
        //设定主8259的中断irq0~irq7由gate0x20~gate0x27接受
    i386_nop3();

    ru_port_write8(I8259_S_DATA_PORT, GATE_INDEX_IRQ8); 
        //设定从8259的中断irq8~irq15由gate0x28~gate0x2f接受
    i386_nop3();

    //主8259与从8259相连
    ru_port_write8(I8259_M_DATA_PORT, 0x04);
    i386_nop3();
    ru_port_write8(I8259_S_DATA_PORT, 0x02);
    i386_nop3();

    //无缓冲区模式
    ru_port_write8(I8259_M_DATA_PORT, 0x01);
    i386_nop3();
    ru_port_write8(I8259_S_DATA_PORT, 0x01);
    i386_nop3();

    //先屏蔽所有中断
    ru_port_write8(I8259_M_DATA_PORT, 0xfb);  //不屏蔽从8259
    i386_nop3();
    ru_port_write8(I8259_S_DATA_PORT, 0xff);
    i386_nop3();

    return STATUS_SUCCESS;
}   

void set_irq_valid(uint16 irq, int32 valid) {
    if(irq >= 0 && irq < 8) {
        uint8 mask = ru_port_read8(I8259_M_DATA_PORT);
        if(valid) {
            mask &= ~(1<<irq);
        }else {
            mask |= (1<<irq);
        }
        ru_port_write8(I8259_M_DATA_PORT, mask);
        i386_nop3();
    }else if(irq >= 8 && irq < 16) {
        irq -= 8;
        uint8 mask = ru_port_read8(I8259_S_DATA_PORT);
        if(valid) {
            mask &= ~(1<<irq);
        }else {
            mask |= (1<<irq);
        }
        ru_port_write8(I8259_S_DATA_PORT, mask);
        i386_nop3();
    }
}