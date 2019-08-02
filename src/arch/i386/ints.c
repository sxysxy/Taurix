/* File: src/arch/i386/ints.c
    init_pic的实现
         author: hfcloud(sxysxygm@gmail.com)
           date: 2019.07.30
 */

#include <taurix/arch/i386/i386_utils.h>

void entry_keyboard_int_handler();
void keyboard_int_handler() {
    ru_text_set_color(VGA_TEXT_GREEN);
    ru_text_print("[ MSG ] Keyboard\n");
}

int32 init_irq_ints() {
    i386_set_idt_item(g_idt + GATE_INDEX_IRQ1, SELECTOR_INDEX_CODE32_KERNEL, (uint32)entry_keyboard_int_handler, FLAGS_INTGATE);
    return STATUS_SUCCESS;
}

#define def_handler_func(fname, excp) void entry_##fname(); \
void fname() {     \
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