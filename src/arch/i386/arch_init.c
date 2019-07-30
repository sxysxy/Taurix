/* File: src/arch/i386/arch_init.c
    目标平台初始化的接口的声明
        author: hfcloud(sxysxygm@gmail.com)

 */

#include <taurix/arch_init.h>
#include <taurix/arch/i386/i386_utils.h>
#include <taurix/mm/basic_mm.h>

GDTItem *g_gdt = 0;
IDTItem *g_idt = 0;

uint32 init_gdt() {
    g_gdt = ru_malloc(sizeof(GDTItem) * GDT_MAX_NUMBER);
    i386_set_gdt_item(g_gdt + 0, 0, 0, 0);
    i386_set_gdt_item(g_gdt + SELECTOR_INDEX_CODE32_KERNEL, 0, 0xffffffffu, FLAGS_GDT_CODE32_KERNEL);
    i386_set_gdt_item(g_gdt + SELECTOR_INDEX_DATA32_KERNEL, 0, 0xffffffffu, FLAGS_GDT_DATA32_KERNEL);
    i386_set_gdt_item(g_gdt + SELECTOR_INDEX_CODE32_USER, 0, 0xffffffffu, FLAGS_GDT_CODE32_USER);
    i386_set_gdt_item(g_gdt + SELECTOR_INDEX_DATA32_USER, 0, 0xffffffffu, FLAGS_GDT_DATA32_USER);
    i386_set_gdtr(sizeof(GDTItem) * GDT_MAX_NUMBER - 1, g_gdt);
    void reload_selectors();
    reload_selectors();
}

uint32 init_idt() {

}

//针对目标平台的设备初始化模块
// 返回 STATUS_SUCCESS 成功
//     STATUS_FAILED
uint32 arch_init() {
    arch_init_memory();
    init_idt();

    return STATUS_SUCCESS;
}

//初始化内存（为高级内存管理做准备）
uint32 arch_init_memory() {
    init_gdt();
}

//初始化储存设备
uint32 arch_init_storage() {

}

//初始化时钟
uint32 arch_init_clock() {

}

//初始化键盘输入设备
uint32 arch_init_keyboard() {

}

//初始化图形设备
uint32 arch_init_video() {

}

//杂项的初始化
uint32 arch_init_misc() {

}
