/* 
 File: src/arch/i386/i386_utils1.c
   i386平台专用，实现include/taurix/arch/i386/i386_utils.h中声明的平台相关的接口
      author: hfcloud(sxysxygm@gmail.com)
        date: 2019.07.30
*/

#include <taurix/arch/i386/i386_utils.h>

/* 设置gdt项
 */
void i386_set_gdt_item(GDTItem *item, uint32 base, uint32 limit, uint32 flags) {
    if(flags & SEL_4K) {
        limit /= 0x1000; 
    }
    
    uint8 flags1 = flags & 0x00ff;
    uint8 flags2 = (flags >> 8) & 0xf0;

    item->base_low = base & 0xffff;
    item->base_middle = (base >> 16) & 0xff;
    item->base_hight - (base >> 24) & 0xff;

    item->limit_low = limit & 0xffff;
    
    item->flags = flags1;
    item->limit_high_with_flags = ((limit >> 16) & 0x0f) | flags2;
}

/* 设置idt项
 */
void i386_set_idt_item(IDTItem *item, uint32 selector, uint32 offset, uint32 flags) {
    item->selector = selector;
    
    item->offset_low = offset & 0xffff;
    item->offset_hight = (offset >> 16) & 0xffff;

    item->flags = flags;
    
    item->reserved = 0;
}