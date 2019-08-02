/* File src/arch/i386/i386_process.c
        进程
            ...
            author: hfcloud(sxysxygm@gmail.com)
              date: 2019.07.31              
 */


#include <taurix/arch/i386/i386_utils.h>
#include <taurix/process.h>
#include <taurix/mm/basic_mm.h>

struct tagProcess {
    ProcessInfo info; //utility
    
    TSS32 tss;
};

Process *process_initialize(ProcessInfo *info) {
    Process *proc = ru_malloc(sizeof(Process));
    ru_memcpy(&proc->info, info, sizeof(ProcessInfo));
    ru_memset(&proc->tss, 0, sizeof(TSS32));
    proc->tss.eip = (uint32)proc->info.entry;
    proc->tss.eflags = 0x00000202u;   
    proc->tss.esp = (uint32)proc->info.stack + proc->info.stack_size;
    proc->tss.cs = SELECTOR_INDEX_CODE32_USER * 8;
    proc->tss.ds = proc->tss.es = proc->tss.fs = proc->tss.gs = SELECTOR_INDEX_DATA32_USER * 8;
    proc->tss.io_permission = 0x40000000;
    
    return proc;
}   

int32 process_switch_to(Process *proc) {
    //当前cpu对应的tss选择子
    uint32 selector = SELECTOR_INDEX_TSS(get_cpu_index()); 
    i386_set_gdt_item(g_gdt + selector, (uint32)&proc->tss, TSS32_LIMIT, FLAGS_TSS32);
    i386_ltr(selector);
    /* 
    proc->tss.eip = (uint32)proc->info.entry;
    proc->tss.eflags = 0x00000202u;   
    proc->tss.esp = (uint32)proc->info.stack + proc->info.stack_size;
    proc->tss.cs = SELECTOR_INDEX_CODE32_USER * 8;
    proc->tss.ds = proc->tss.es = proc->tss.fs = proc->tss.gs = SELECTOR_INDEX_DATA32_USER * 8;
    */
    //i386_jmp_sel(selector, 0);
    return STATUS_SUCCESS;
}

int32 process_get_info(Process *proc, ProcessInfo *pinfo) {
    ru_memcpy(pinfo, &proc->info, sizeof(ProcessInfo));
    return STATUS_SUCCESS;
}
