/*
  文件: src/ipc.c 进程间通信
        author: hfcloud(sxysxygm@gmail.com)
          date: 2019.08.27
*/

#include <taurix/mm/basic_mm.h>
#include <taurix/ipc.h>

int32 ipc_send_impl(TMessage *msg, Process *receiver) {
    ProcessInfo *ri = (ProcessInfo*)receiver;
    ProcessInfo *si = (ProcessInfo*)ps_get_working_scheduler()->current;

    if(ri->status == PROCESS_STATUS_RECEIVING) {
        ru_memcpy(ri->queuing_message, msg, sizeof(TMessage));
        ps_unblock_process(ps_get_working_scheduler(), ri->pid);
    } else {
        ProcessQueuer *queuer = ru_malloc(sizeof(ProcessQueuer));
        queuer->next = NULL;
        queuer->process_id = msg->sender_pid;
        if(ri->queuing_list) {
            ri->queuing_tail->next = queuer;
            ri->queuing_tail = queuer;
        } else {
            ri->queuing_list = ri->queuing_tail = queuer;
        }
    }
    return STATUS_SUCCESS;
}

int32 ipc_recv_impl(TMessage *msg) {
    ProcessScheduler *ps = ps_get_working_scheduler();
    ProcessInfo *pi = (ProcessInfo*)ps->current;  //current
    
    if(pi->queuing_list == NULL) {
        ps_block_process(ps, pi->pid, PROCESS_STATUS_RECEIVING);
    } else {
        ProcessQueuer *queuer = pi->queuing_list;
        pi->queuing_list = queuer->next;
        
        //TODO: make memory shared
        Process *sender;
        ps_get_process(ps_get_working_scheduler(), queuer->process_id, &sender);
        ru_memcpy(msg, ((ProcessInfo*)sender)->queuing_message, sizeof(TMessage));
        ru_free(queuer);
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