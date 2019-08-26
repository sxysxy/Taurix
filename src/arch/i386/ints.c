/* File: src/arch/i386/ints.c
    init_pic的实现
         author: hfcloud(sxysxygm@gmail.com)
           date: 2019.07.30
 */

#include <taurix/arch/i386/i386_utils.h>

void entry_keyboard_int_handler() EXPORT_SYMBOL(entry_keyboard_int_handler);
void keyboard_int_handler(TContext *Context) EXPORT_SYMBOL(keyboard_int_handler);
void keyboard_int_handler(TContext *context) {
    static const char *scan_code_table[] = {
        "0x00", "Esc", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "=", "Backspace", "Tab", 
        /* 0x10 */"Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "left bracket", "right bracket", "Enter", "left ctrl", 
        /* 0x1e */"A", "S", "D", "F", "G", "H", "J", "K", "L", ";", "'", "'", "left shift", "\\",
        /* 0x2c */"Z", "X", "C", "V", "B", "N", "M", ",", ".", "/", "right shift", "*", "left alt", "0x39", "0x3a",
        /* 0x3b */"F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "Num", "0x46",
        /* 0x47 */"7", "8", "9",  "-", "4", "5", "6", "+", "1", "2", "3", "0", ".", "0x54", "0x55", "0x56",
        /* 0x57 */"F11", "F12", "unsupported"
    };

    ru_text_set_color(VGA_TEXT_GREEN);
    char tmp[100];
    uint8 scan_code = ru_port_read8(0x60);
    if(scan_code > 0x58 && !(scan_code & 0x80)) {
        scan_code = 0x59;
    }
    ru_sprintf_s(tmp, 100, "[ MSG ] Keyboard: %s | %s\n", scan_code_table[scan_code & ~(0x80)], (scan_code & 0x80) ? "UP" : "DOWN");
    ru_text_print(tmp);
    ru_port_write8(0x20, 0x61);
    i386_nop3();
}

int32 init_irq_ints() {
    i386_set_idt_item(g_idt + GATE_INDEX_IRQ1, SELECTOR_INDEX_CODE32_KERNEL, (uint32)entry_keyboard_int_handler, FLAGS_INTGATE);
    return STATUS_SUCCESS;
}

#define def_handler_func(fname, excp) void entry_##fname() EXPORT_SYMBOL(entry_##fname); \
void fname(TContext *context) EXPORT_SYMBOL(fname); \
void fname(TContext *context) {     \
    ru_text_set_color(VGA_TEXT_RED);                     \
    ru_text_print(excp); \
    ru_kernel_suspend(); \
}

def_handler_func(exception_de_handler, "[ Exception ] Divided by zero\n")
def_handler_func(exception_df_handler, "[ Exception ] Double Fault\n")
def_handler_func(exception_gp_handler, "[ Exception ] General Protection\n")

int32 init_exception_ints() {
    i386_set_idt_item(g_idt + GATE_INDEX_EXCEPTION_DE, SELECTOR_INDEX_CODE32_KERNEL, (uint32)entry_exception_de_handler, FLAGS_INTGATE);
    i386_set_idt_item(g_idt + GATE_INDEX_EXCEPTION_DF, SELECTOR_INDEX_CODE32_KERNEL, (uint32)entry_exception_de_handler, FLAGS_INTGATE);
    i386_set_idt_item(g_idt + GATE_INDEX_EXCEPTION_GP, SELECTOR_INDEX_CODE32_KERNEL, (uint32)entry_exception_gp_handler, FLAGS_INTGATE);
    return STATUS_SUCCESS;
}

int32 init_ints() {

    //TODO: other ints
    init_irq_ints();
    init_exception_ints();

    return STATUS_SUCCESS;
}