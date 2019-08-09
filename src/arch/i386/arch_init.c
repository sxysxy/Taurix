/* File: src/arch/i386/arch_init.c
    目标平台初始化的接口的声明
        author: hfcloud(sxysxygm@gmail.com)

 */

#include <taurix/arch_init.h>
#include <taurix/arch/i386/i386_utils.h>
#include <taurix/mm/basic_mm.h>

GDTItem *g_gdt = 0;
IDTItem *g_idt = 0;

void reload_selectors() EXPORT_SYMBOL(reload_selectors);
int32 init_gdt() {
    g_gdt = ru_malloc(sizeof(GDTItem) * GDT_MAX_NUMBER);
    if((uint32)g_gdt % 8) 
        g_gdt = (GDTItem *)(((uint32)g_gdt) + 4);
    ru_memset(g_gdt, 0, sizeof(GDT_MAX_NUMBER) * GDT_MAX_NUMBER);
    i386_set_gdt_item(g_gdt + 0, 0, 0, 0);
    i386_set_gdt_item(g_gdt + SELECTOR_INDEX_CODE32_KERNEL, 0, 0xffffffffu, FLAGS_GDT_CODE32_KERNEL);
    i386_set_gdt_item(g_gdt + SELECTOR_INDEX_DATA32_KERNEL, 0, 0xffffffffu, FLAGS_GDT_DATA32_KERNEL);
    i386_set_gdt_item(g_gdt + SELECTOR_INDEX_CODE32_USER, 0, 0xffffffffu, FLAGS_GDT_CODE32_USER);
    i386_set_gdt_item(g_gdt + SELECTOR_INDEX_DATA32_USER, 0, 0xffffffffu, FLAGS_GDT_DATA32_USER);
    i386_set_gdtr(sizeof(GDTItem) * GDT_MAX_NUMBER - 1, g_gdt);
    
    reload_selectors();
    return STATUS_SUCCESS;
}

int32 init_idt() {
    g_idt = ru_malloc(sizeof(IDTItem) * (IDT_MAX_NUMBER)+1);
    if((uint32)g_idt % 8)
        g_idt = (IDTItem *)(((uint32)g_idt) + 4);  //align 8
    ru_memset(g_idt, 0, sizeof(IDT_MAX_NUMBER) * IDT_MAX_NUMBER);
    i386_set_idtr(sizeof(IDTItem) * IDT_MAX_NUMBER - 1, g_idt);
    return STATUS_SUCCESS;
}

int32 arch_init_memory();

//针对目标平台的设备初始化模块
// 返回 STATUS_SUCCESS 成功
//     STATUS_FAILED
int32 arch_init() {
    arch_init_memory();
    init_pic();
    arch_init_clock();
    arch_init_keyboard();
    arch_init_storage();
    arch_init_video();
    arch_init_misc();
    init_idt();
    init_ints();

    return STATUS_SUCCESS;
}

//初始化内存（为高级内存管理做准备）
int32 arch_init_memory() {
    init_gdt(); 

    return STATUS_SUCCESS;
}

//初始化储存设备
int32 arch_init_storage() {

    return STATUS_SUCCESS;
}

//初始化时钟
int32 arch_init_clock() {
    //交由进程调度器处理
    return STATUS_SUCCESS;
}

//初始化键盘输入设备
int32 arch_init_keyboard() {
    set_irq_valid(PIC_IRQ_KEYBOARD, 1);
    return STATUS_SUCCESS;
}

//初始化图形设备
int32 arch_init_video() {

    return STATUS_SUCCESS;
}

//杂项的初始化
int32 arch_init_misc() {

    return STATUS_SUCCESS;
}
