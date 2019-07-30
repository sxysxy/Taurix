/* File: src/arch/i386/ints.c
    init_pic的实现
         author: hfcloud(sxysxygm@gmail.com)
           date: 2019.07.30
 */

#include <taurix/arch/i386/i386_utils.h>

int32 init_irq_ints() {
    return STATUS_SUCCESS;
}

int32 init_ints() {

    //TODO: other ints
    init_irq_ints();
    return STATUS_SUCCESS;
}