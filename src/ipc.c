/*
  文件: src/ipc.c 进程间通信
        author: hfcloud(sxysxygm@gmail.com)
          date: 2019.08.27
*/

#include <taurix/ipc.h>

int32 ipc_send_impl(TMessage *msg, Process *receiver) {
    return STATUS_SUCCESS;
}

int32 ipc_recv_impl(TMessage *msg) {

    return STATUS_SUCCESS;
}

int32 ipc_post(TMessage *msg, Process *receiver);

int32 ipc_peek(TMessage *msg);