/* File: src/arch/i386/arch_utils1.c
    i386平台相关的一些基本公用函数的实现，对应include/taurix/utils.h中的一部分函数的声明

    主要包括： 基本的字符输出
             根据multiboot信息实现ru_detect_available_memory

    author: hfcloud(sxysxygm@gmail.com) 
        date: 2019.07.29
 */

#include <taurix.h>

typedef struct tagTextChar {
    union {
        uint16 unit;
        struct {
            uint8 ch;
            uint8 color;
        };
    };
}TextChar;

static TextChar *vram;
static int32 cursor_row, cursor_col, ROWS, COLUMNS; 
static int32 text_color;

#define pos2ptr(row, col) (vram + row*COLUMNS + col)

/*ru_text_init
 */
int32 ru_text_init() {
    vram = (TextChar*)0xb8000;
    ROWS = ru_text_get_rows();
    COLUMNS = ru_text_get_colmns();
    cursor_row = 0;
    cursor_col = 0;
    text_color = 7; //白色white
    return STATUS_SUCCESS;
}

/*ru_text_set_color
 */
void ru_text_set_color(uint32 color) {
    text_color = color;
}

/*ru_text_get_columns
 */
int32 ru_text_get_colmns() {
    return 80;
}

/*ru_text_get_rows
 */
int32 ru_text_get_rows() {
    return 25;
}

/*ru_text_putchar
 */
int32 ru_text_putchar(int ch) {
    int old_row = cursor_row, old_col = cursor_col;
    if(ch == '\n' || ++cursor_col >= COLUMNS) {
        cursor_row += 1;
        cursor_col = 0;
        if(cursor_row >= 25) {
            ru_disable_interrupt();
            
            if(cursor_row > ROWS * 8) {  //写了8页，清空重新写
                cursor_row = 0;
                ru_port_write8(0x3d4, 0x0d);
                ru_port_write8(0x3d5, 0);
                ru_port_write8(0x3d4, 0x0c);
                ru_port_write8(0x3d5, 0);
                ru_enable_interrupt();
                ru_memset(vram, 0, sizeof(TextChar) * COLUMNS * ROWS * 8);
                return ru_text_putchar(ch);
            }

            uint32 pos = (80*(cursor_row-25));

            ru_port_write8(0x3d4, 0x0d);
            ru_port_write8(0x3d5, pos & 0xff);
            ru_port_write8(0x3d4, 0x0c);
            ru_port_write8(0x3d5, (pos >> 8) & 0xff);

            ru_enable_interrupt();
        }
    }
    if(ch == '\n')
        return 0;
    TextChar *p = pos2ptr(old_row, old_col);
    p->ch = ch;
    p->color = text_color;
    return ch;
}

/*ru_text_print
 */
void ru_text_print(const char *text) {
    while(*text) {
        ru_text_putchar(*text++);
    }
}

/*ru_text_clear 
*/
void ru_text_clear() {
    for(int i = 0; i < COLUMNS; i++) {
        for(int j = 0; j < ROWS; j++) {
            pos2ptr(i, j)->unit = 0;
        }
    }
}

/* ru_detect_available_memory
 */
typedef struct tagMultibootInfo {
    uint32 flags;
                //内存域，单位为kb
    uint32 mem_lower;   //低端内存的大小，基地址为0
    uint32 mem_upper;   //高端内存的大小，基地址为1mb

    //TODO: 如果有其它需要再添加
}MultibootInfo;

extern MultibootInfo *g_multiboot_info_addr EXPORT_SYMBOL(g_multiboot_info_addr);  //定义在boot.S中

int32 ru_detect_available_memory(size_t min_size, _OUT void **baseaddr, _OUT size_t *actual_size) {
    MultibootInfo *mbinfo = g_multiboot_info_addr;
    /* 
    char tmp[100];
    ru_text_set_color(2);
    ru_sprintf_s(tmp, 100, "mbinfo addr: %p\n", mbinfo);
    ru_text_print(tmp);
    ru_sprintf_s(tmp, 100, "mem_lower: %d, mem_upper: %d\n", mbinfo->mem_lower, mbinfo->mem_upper);
    ru_text_print(tmp);
    */
    if(mbinfo->mem_upper > (min_size >= 1024 ? (min_size / 1024 + 1) : 1)) {  //ok
        *baseaddr = (void*)(1*1024*1024);
        uint64 size = mbinfo->mem_upper * 1024; //换算成字节，uint32可能会溢出，使用uint64储存
        if(size > 4294967296llu-(uint32)baseaddr) 
            size = 4294967296llu-(uint32)baseaddr;
        *actual_size = (uint32)size; //对于i386, sizeof(size_t) == sizeof(uint32)
        return STATUS_SUCCESS;
    } else {
        *baseaddr = 0;
        *actual_size = 0;
        return STATUS_FAILED;
    }
}

