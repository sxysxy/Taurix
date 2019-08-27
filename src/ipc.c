/*
  文件: src/ipc.c 进程间通信
        author: hfcloud(sxysxygm@gmail.com)
          date: 2019.08.27
*/

#include <taurix/ipc.h>

int32 ipc_send_impl(TMessage *msg, Process *receiver) {
    ProcessInfo* ri = (ProcessInfo*)receiver;

    ps_unblock_process(ps_get_working_scheduler(), ri->pid);    
    ps_immidate_reschedule();
    return STATUS_SUCCESS;
}

int32 ipc_recv_impl(TMessage *msg) {
    ProcessScheduler *ps = ps_get_working_scheduler();
    ProcessInfo *pi = (ProcessInfo*)ps->current;  //current
    if(pi->queuing_list == NULL) {
        ps_block_process(ps, pi->pid, PROCESS_STATUS_RECEIVING);
        ps_immidate_reschedule();
    } else {
        ProcessQueuer *queuer = pi->queuing_list;
        pi->queuing_list = queuer->next;
        
        //TODO: make memory shared
        ru_memcpy(msg, queuer->message, sizeof(TMessage));
    }
    return STATUS_SUCCESS;
}

int32 ipc_notify_impl(TMessage *msg) {
    ProcessScheduler *ps = ps_get_working_scheduler();
    ps_unblock_process(ps, msg->sender_pid);
    return STATUS_SUCCESS;
}

int32 ipc_post_impl(TMessage *msg, Process *receiver);

int32 ipc_peek_impl(TMessage *msg);