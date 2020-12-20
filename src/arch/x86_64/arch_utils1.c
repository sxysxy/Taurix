/* File: src/arch/x86_64/arch_utils1.c
    x86_64平台相关的一些基本公用函数的实现，对应include/taurix/utils.h中的一部分函数的声明
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

/*ru_text_get_columns
 */
int32 ru_text_get_colmns() {
    return 80;
}

/*ru_text_get_rows
 */
int32 ru_text_get_rows() {
    return 24;
}

/*ru_text_putchar
 */
int32 ru_text_putchar(int ch) {
    int old_row = cursor_row, old_col = cursor_col;
    if(ch == '\n' || ++cursor_col >= COLUMNS) {
        cursor_row += 1;
        cursor_col = 0;
    }
    if(ch == '\n')
        return;
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

/*ru_text_printf
 */
int32 ru_text_printf(const char *format, ...) {
    //TODO
    return 0;
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

