/* File: include/arch/i386/ints.h
    早期中断处理程序
        author: hfcloud(sxysxygm@gmail.com)
          date: 2019.07.30
 */

#ifndef INITS_H
#define INITS_H

#include <taurix.h>

CSTART 

#define GATE_INDEX_IRQ0         0x20
#define GATE_INDEX_IRQ1         0x21
#define GATE_INDEX_IRQ2         0x22
#define GATE_INDEX_IRQ3         0x23
#define GATE_INDEX_IRQ4         0x24
#define GATE_INDEX_IRQ5         0x25
#define GATE_INDEX_IRQ6         0x26
#define GATE_INDEX_IRQ7         0x27
#define GATE_INDEX_IRQ8         0x28
#define GATE_INDEX_IRQ9         0x29
#define GATE_INDEX_IRQ10        0x2a
#define GATE_INDEX_IRQ11        0x2b
#define GATE_INDEX_IRQ12        0x2c
#define GATE_INDEX_IRQ13        0x2d
#define GATE_INDEX_IRQ14        0x2e
#define GATE_INDEX_IRQ15        0x2f

//Divide Error(caused by div 0)
#define GATE_INDEX_EXCEPTION_DE   0x00

//Double Fault
#define GATE_INDEX_EXCEPTION_DF   0x08

//General Protection
#define GATE_INDEX_EXCEPTION_GP   0x0d

//初始化中断 */
int32 init_ints();

CEND

#endif
