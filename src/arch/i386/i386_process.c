/* File src/arch/i386/i386_process.c
        进程
        不使用i386平台上的gdt,idt,tss实现进程切换
            ...
            author: hfcloud(sxysxygm@gmail.com)
              date: 2019.07.31              
        完善进程调度模块(by hfcloud, 2019.08.03)
 */


#include <taurix/arch/i386/i386_utils.h>
#include <taurix/process.h>
#include <taurix/utils/linklist.h>
#include <taurix/mm/basic_mm.h>

struct tagProcess {
    ProcessInfo info; //utility

    TContext context;  //用于切换任务时的上下文
    
    /* 不再使用i386平台提供的机制 
    uint16 ldt_selector;
    GDTItem ldts[2];
    TSS32 tss;
    */
};

size_t process_query_sizeof_process() {
    return sizeof(struct tagProcess);
}

int32 process_initialize(Process *proc, ProcessInfo *info) {
    ru_memcpy(&proc->info, info, sizeof(ProcessInfo));
    //ru_memset(&proc->tss, 0, sizeof(TSS32));
    /* //init ldts:
    ru_memcpy(proc->ldts+0, g_gdt + SELECTOR_INDEX_CODE32_KERNEL, sizeof(GDTItem));
    ru_memcpy(proc->ldts+1, g_gdt + SELECTOR_INDEX_DATA32_KERNEL, sizeof(GDTItem));
    */

    if((info->flags & 0xff) > 7) {
        //非法参数
        return STATUS_FAILED;
    }
    
    uint16 sel_code = info->flags & PROCESS_PRIVILEGE_KERNEL ? (SELECTOR_INDEX_CODE32_KERNEL * 8) : (SELECTOR_INDEX_CODE32_USER * 8);
    uint16 sel_data = info->flags & PROCESS_PRIVILEGE_KERNEL ? (SELECTOR_INDEX_DATA32_KERNEL * 8) : (SELECTOR_INDEX_DATA32_USER * 8);

    ru_memset(&proc->context, 0, sizeof(TContext));
    proc->context.ss0 = sel_data;
    proc->context.esp0 = (uint32)proc->info.stack + proc->info.stack_size;
    proc->context.eflags = 0x00000202u;   
    proc->context.cs = sel_code;
    proc->context.eip = (uint32)proc->info.entry;
    proc->context.ds = proc->context.es = proc->context.fs = proc->context.gs = sel_data;
    
    return STATUS_SUCCESS;
}   

void simulate_iret(TContext context) EXPORT_SYMBOL(simulate_iret);
int32 process_switch_to(Process *target, uint32 flags) {
    //自力更生，丰衣足食，破TSS一点都不好用，软件方法实现进程切换就完事了
    //if(flags & PROCESS_SCHEDULE_FROM_REQ) {    //软件请求进入调度或未有当前运行中进程
        simulate_iret(target->context);
    //    return STATUS_SUCCESS;
    //} 
    //return STATUS_FAILED;
}

int32 process_get_info(Process *proc, ProcessInfo *pinfo) {
    ru_memcpy(pinfo, &proc->info, sizeof(ProcessInfo));
    return STATUS_SUCCESS;
}

//初始化
int32 ps_initialize(ProcessScheduler *ps, uint32 max_process) {
    ps->proc_table = ru_malloc(process_query_sizeof_process() * max_process);
    ru_memset(ps->proc_table, 0, process_query_sizeof_process() * max_process);
    ps->max_process = max_process;
    ps->current = NULL;
}

//添加进程，失败会返回STATUS_FAILED
int32 ps_add_process(ProcessScheduler *ps, ProcessInfo *info) {
    if(!info || (info->flags & PROCESS_PRESENT) == 0) {
        return STATUS_FAILED;
    }
    for(uint32 i = 0; i < ps->max_process; i++) {
        if ((ps->proc_table[i].info.flags & PROCESS_PRESENT) == 0) { //表中空闲的表项
            process_initialize(&ps->proc_table[i], info);
            ps->proc_table[i].info.pid = i;
            ps->proc_table[i].info.scheduler = ps;
            ps->proc_table[i].info.queuing_list = NULL;
            return STATUS_SUCCESS;
        }
    }
    return STATUS_FAILED;
}

static ProcessScheduler *g_current_ps;
void *entry_clock_int_handler() EXPORT_SYMBOL(entry_clock_int_handler);
void ps_do_auto_schedule(ProcessScheduler *ps, TContext *context);
void clock_int_handler(TContext *context) EXPORT_SYMBOL(clock_int_handler);
void clock_int_handler(TContext *context) {
    ps_do_auto_schedule(g_current_ps, context);
    ru_port_write8(0x20, 0x60);
}

//开始进行调度, 使用第一个加入调度器的进程作为第一个进程，duration_per_slice指定了每个时间片的时间（单位：ms）
int32 ps_schedule(ProcessScheduler *ps, uint32 duration_per_slice) { 
    ru_disable_interrupt();
    //
    uint16 freq = 1000 / duration_per_slice;
    uint16 magic = freq / 100 * 11932;
    ru_port_write8(0x43, 0x34);
    ru_port_write8(0x40, (magic>>8) & 0xff);
    i386_nop3();
    ru_port_write8(0x40, magic & 0xff); 
    i386_nop3();

    //
    i386_set_idt_item(g_idt + GATE_INDEX_IRQ0, SELECTOR_INDEX_CODE32_KERNEL, (uint32)entry_clock_int_handler, FLAGS_INTGATE);
    set_irq_valid(PIC_IRQ_CLOCK, 1);
    ps->current = NULL;
    g_current_ps = ps;
    ru_enable_interrupt();
    /* 
    if(ps->proc_table[0].info.flags & PROCESS_PRESENT) {
        ps->current = &ps->proc_table[0];
        process_switch_to(&ps->proc_table[0], PROCESS_SCHEDULE_FROM_REQ);
    }
    */
}

//给时钟/定时器中断调用的
//注意，参数的ps不能用，如果直接把ps_do_auto_schedule作为中断处理程序，参数ps不可用，参数ps实际上是Context
void ps_do_auto_schedule(ProcessScheduler *ps, TContext *context) {  //时间片轮转算法
    Process *perfer_proc = NULL;
    if(ps->current) {
        ru_memcpy(&ps->current->context, context, sizeof(TContext));
        ps->current->info.status = PROCESS_STATUS_READY;
    }
    int max_priority = 0;
    for(int i = 0; i < ps->max_process; i++) {  //选出当前剩余时间片最多的进程
        Process *proc = &ps->proc_table[i];
        if(!proc || (proc->info.flags & PROCESS_PRESENT) == 0 || proc->info.status > PROCESS_STATUS_RUNNABLE) continue;
        if(proc->info.remain_time_slice <= 0)continue;
        if(proc->info.remain_time_slice > max_priority) {
            perfer_proc = proc;
            max_priority = proc->info.remain_time_slice;
        } 
    }
    if(!perfer_proc) {  //如果找不到，重置每一个进程的剩余时间片数量为其优先级的大小，重新选出剩余时间片最多的进程
        max_priority = 0;
        for(int i = 0; i < ps->max_process; i++) {
            Process *proc = &ps->proc_table[i];
            if(!proc || (proc->info.flags & PROCESS_PRESENT) == 0 || proc->info.status > PROCESS_STATUS_RUNNABLE) continue;
            proc->info.remain_time_slice = proc->info.priority;
            if(proc->info.remain_time_slice > max_priority) {
                perfer_proc = proc;
                max_priority = proc->info.remain_time_slice;
            } 
        }
    }
    if(perfer_proc) {
        
        //切换当前进程
        perfer_proc->info.remain_time_slice--;
        ps->current = perfer_proc;

        if(perfer_proc->info.status == PROCESS_STATUS_READY) 
            perfer_proc->info.status = PROCESS_STATUS_RUNNING;

        ru_memcpy(context, &ps->current->context, sizeof(TContext)); 
    } else { // //依然没有进程可以调度，挂起
    /*
        ru_text_set_color(VGA_TEXT_RED);
        ru_text_print("[ Halt ] No process to switch\n");
        ru_kernel_suspend();
    */
    }
}

//阻塞进程
int32 ps_block_process(ProcessScheduler *ps, uint32 pid, uint32 flag) {
    if(pid < 0 || pid >= ps->max_process) 
        return STATUS_FAILED;
    Process *prcess = &ps->proc_table[pid];
    prcess->info.status = flag;
    return STATUS_SUCCESS;
}

//解除进程阻塞
int32 ps_unblock_process(ProcessScheduler *ps, uint32 pid) {
    if(pid < 0 || pid >= ps->max_process)
        return STATUS_FAILED;
    ps->proc_table[pid].info.status = PROCESS_STATUS_READY;
    return STATUS_SUCCESS;
}

//死锁检查
int32 ps_check_deadlock(ProcessScheduler *ps, uint32 pid) {
     if(pid < 0 || pid >= ps->max_process)
        return 1;
    ProcessInfo *pi = (ProcessInfo*) &ps->proc_table[pid];
    ProcessQueuer *p = pi->queuing_list;
    /*
    while(p) {

    }*/
    return 0;
}

ProcessScheduler *ps_get_working_scheduler() {
    return g_current_ps;
}

int32 ps_get_process(ProcessScheduler *ps, uint32 pid, Process **process) {
    if(pid < 0 || pid >= ps->max_process) {
        *process = NULL;
        return STATUS_FAILED;
    }
    *process = &ps->proc_table[pid];
    return STATUS_SUCCESS;
}

void ps_immdiate_reschedule(void *context) {
    ProcessScheduler *ps = ps_get_working_scheduler();
    ps_do_auto_schedule(ps, context);
    ru_memcpy(context, &ps->current->context, sizeof(TContext));
}

void ps_exit_process(uint32 exit_code) {
    ProcessScheduler *ps = ps_get_working_scheduler();
    ps->current->info.exit_code = exit_code;
    ps->current->info.flags &= (~PROCESS_PRESENT);
    ps_immdiate_reschedule(&ps->current->context);
    return;
}