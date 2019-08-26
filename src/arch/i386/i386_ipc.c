/*  src/arch/i386/i386_ipc.c
        i386平台上IPC实现
        系统调用ipc_send(eax = 0), ipc_recv(eax = 1)，ipc_post(eax = 2)，ipc_peek(eax = 3)
            ebx为指向TMessage的指针
            ecx为接受消息的进程的pid(对于send和post)
*/

#define IPC_INT_GATE_INDEX      (0x9f)

#include <taurix/arch/i386/i386_utils.h>
#include <taurix/ipc.h>
#include <taurix/mm/basic_mm.h>

void entry_ipc_syscall() EXPORT_SYMBOL(entry_ipc_syscall);
void ipc_syscall(TContext *context) EXPORT_SYMBOL(ipc_syscall);

void ipc_syscall(TContext *context) {
    ProcessScheduler *ps = ps_get_working_scheduler();
    TMessage *msg = (TMessage*)context->ebx;
    uint32 sender_pid = ((ProcessInfo*)ps->current)->pid;
    switch (context->eax)
    {
    case 0:         //send
        {
        uint32 receiver_pid = context->ecx;
        if(receiver_pid < 0 || receiver_pid >= ps->max_process) {
            return;
        }
        Process *receiver = NULL;
        ps_get_process(ps, receiver_pid, &receiver);
        if(!receiver) return;

        ProcessBlockList *blocker = (ProcessBlockList *)ru_malloc(sizeof(ProcessBlockList));
        blocker->next = NULL;
        blocker->process_id = receiver_pid;
        
        ps_block_process(ps, sender_pid, blocker, PROCESS_STATUS_SENDING);
        
        if(ps_check_deadlock(ps, sender_pid)) {
            if(((ProcessInfo*)ps->current)->flags & PROCESS_PRIVILEGE_KERNEL) {
                ru_text_set_color(VGA_TEXT_RED);
                ru_text_print("[ Fatal Error ] Kernel Deadlocked\n");
                ru_kernel_suspend();
            }
        }
        
        ipc_send_impl(msg, receiver);

        }
        break;     

    case 1:         //receive
        ipc_recv_impl(msg);
        break;
    
    default:
        /*
        ru_text_set_color(VGA_TEXT_RED);
        ru_text_print("Invalid ipc call argument, ignored\n");
        */
        return;
        break;
    }
}


void init_ipc() {
    i386_set_idt_item(g_idt + IPC_INT_GATE_INDEX, SELECTOR_INDEX_CODE32_KERNEL, (uint32)ipc_syscall, FLAGS_INTGATE);    
}