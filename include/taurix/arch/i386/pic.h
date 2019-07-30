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

CEND

#endif