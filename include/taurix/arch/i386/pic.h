/* File: include/arch/i386/pic.h
        可编程控制器(Programable Interrupt Controller)
        假定只存在两个8259A芯片，主(M)和从(S)
        author: hfcloud(sxysxygm@gmail.com)
          date: 2019.07.30
 */

#ifndef PIC_H
#define PIC_H

#include <taurix.h>
CSTART

//主8259 控制端口
#define I8259_M_CTL_PORT      0x20
//主8259 数据端口
#define I8259_M_DATA_PORT     0x21

//从8259 控制端口
#define I8259_S_CTL_PORT      0xa0 
//从8259 数据端口
#define I8259_S_DATA_PORT     0xa1

/* 可编程中断控制器的初始化
 */
int32 init_pic();

//设定外部中断是否允许发生
void set_irq_valid(uint16 irq, int32 valid);

#define PIC_IRQ_CLOCK             0x00
#define PIC_IRQ_KEYBOARD          0x01
#define PIC_IRQ_I8259_SLAVE       0x02
#define PIC_IRQ_COM2              0x03
#define PIC_IRQ_COM1              0x04
#define PIC_IRQ_LPT2              0x05
#define PIC_IRQ_FDD               0x06
#define PIC_IRQ_LPT1              0x07
#define PIC_IRQ_CMOS_ALERT        0x08
#define PIC_IRQ_I8259_MASTER      0x09
#define PIC_IRQ_RESERVED0         0x0a
#define PIC_IRQ_RESERVED1         0x0b
#define PIC_IRQ_PS2MOUSE          0x0c
#define PIC_IRQ_FPU               0x0d
#define PIC_IRQ_IDE0              0x0e
#define PIC_IRQ_IDE1              0x0f

CEND

#endif