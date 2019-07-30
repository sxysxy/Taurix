/* File: include/taurix/arch/i386/i386_utils.h
    i386平台相关的事物的定义。接口函数实现在src/arch/i386/i386_utils(n).c|.S
    主要包括GDT，IDT结构的定义
    author: hfcloud(sxysxygm@gmail.com)
          date: 2019.07.30
 */

#ifndef I386_UTILS_H
#define I386_UTILS_H

#ifndef __I386__
#define __I386__
#endif


#include <taurix.h>

CSTART

typedef struct tagGDTItem {
    uint16 limit_low, base_low;
    uint8 base_middle;
    
    /* flags:
        type(0-3)
        S(4)  存储段S=1, 系统段描述符/门描述符S=0
        DPL(5-6) 特权级(ring0~ring3)
        P(7)     有效位,P=1时该描述符有效
     */
    uint8 flags;
    
    /* limit_high_with_flags:
        limit_high(0-3)
        available for software(4)
        0(5)
        D(6)
        G(7)
     */
    uint8 limit_high_with_flags;
    uint8 base_hight;
}GDTItem;

typedef struct tagIDTItem {
    //offset为入口函数的偏移地址
    //selector为入口函数所处代码段的选择子
    uint16 offset_low, selector; 
    uint8 reserved;

    /*  8位的flags:
        type(0-2), D(3), 0(4), DPL(5-6), P(7)
        type指定门类型， 任务门，中断门，陷阱门
        D=1 32位, D=0 16位
        DPL指定描述符的特权级(取值0,1,2,3)
        P指定内存中是否存在(1,0)
    */
    uint8 flags; 
    uint16 offset_hight;
}IDTItem;

extern GDTItem *g_gdt;
extern IDTItem *g_idt;

#define GDT_MAX_NUMBER 256
#define IDT_MAX_NUMBER 256

//段已访问
#define ACESS_ACESS         0x01 

//数据段只读
#define ACCESS_RO           0x00

//数据段可读可写
#define ACCESS_RW           0x02

//数据段向下扩展（可以用于栈段）
#define ACCESS_GROWDOWN     0x04 

//代码段，只执行
#define ACESS_EXECO         0x08

//代码段，可读可执行
#define ACCESS_REXEC        0x0a

//代码段，一致段
#define ACCESS_FIT          0x04

//选择子类型，存储段
#define SEL_STORAGE         (1<<4)

//选择子类型，系统段/门描述符
#define SEL_SYS             (0)

//选择子有效
#define SEL_P               (1<<7)

//flags D 位
#define SEL_BITS_32     ((1<<6)<<8)

//flags G bit
#define SEL_4K          ((1<<7)<<8)

//任务门
#define GATE_TASK        (5)
//中断门
#define GATE_INT         (6)
//陷阱门
#define GATE_TRAP        (7)
//32位门
#define GATE_BITS_32     (1<<3)
//门的有效位
#define GATE_P           (1<<7)

//针对DPL域
#define DPL_RING0        (0)
#define DPL_RING1        (1<<5)
#define DPL_RING2        (2<<5)
#define DPL_RING3        (3<<5)

#define PL_RING0          0
#define PL_RING1          1
#define PL_RING2          2
#define PL_TING3          3

//内核代码段
#define SELECTOR_INDEX_CODE32_KERNEL      1
#define FLAGS_GDT_CODE32_KERNEL        (ACCESS_REXEC | SEL_P | SEL_BITS_32 | SEL_4K | SEL_STORAGE | DPL_RING0)

//内核数据段
#define SELECTOR_INDEX_DATA32_KERNEL      2
#define FLAGS_GDT_DATA32_KERNEL        (ACCESS_RW | SEL_P | SEL_BITS_32 | SEL_4K | SEL_STORAGE | DPL_RING0)

//用户代码段
#define SELECTOR_INDEX_CODE32_USER        3
#define FLAGS_GDT_CODE32_USER          (ACCESS_REXEC | SEL_P | SEL_BITS_32 | SEL_4K | SEL_STORAGE | DPL_RING3)

//用户数据段
#define SELECTOR_INDEX_DATA32_USER        4
#define FLAGS_GDT_DATA32_USER          (ACCESS_RW | SEL_P | SEL_BITS_32 | SEL_4K | SEL_STORAGE | DPL_RING3)


/* 设置gdt表
    limit是gdt表最后一个字节的地址
    gdt_addr是gdt表的首地址
 */
void i386_set_gdtr(uint16 limit, void *gdt_addr);

/* 设置gdt项
 */
void i386_set_gdt_item(GDTItem *item, uint32 base, uint32 limit, uint32 access);

/* 设置idt表
    limit是idt表最后一个字节的地址
    gdt_addr是idt表的首地址
 */
void i386_set_idtr(uint16 limit, void *idt_addr);

/* 设置idt项
 */
void i386_set_idt_item(IDTItem *item, uint32 selector, uint32 offset, uint32 flags);

//读取cr0寄存器
uint32 i386_get_cr0();

//设置cr0寄存器值
void i386_set_cr0(uint32 cr0);

//读取cr2寄存器
uint32 i386_get_cr2();

//设置cr2寄存器值
void i386_set_cr2(uint32 cr2);

//读取cr3寄存器
uint32 i386_get_cr3();

//设置cr3寄存器值
void i386_set_cr3(uint32 cr3);

//读取cr4寄存器
uint32 i386_get_cr4();

//设置cr4寄存器值
void i386_set_cr4(uint32 cr4);

//三个nop用于极短暂的延时
void i386_nop3();

CEND


//additional 
#include <taurix/arch/i386/pic.h>
#include <taurix/arch/i386/ints.h>

#endif