/* File include/taurix/ipc.h
        IPC(inner process communication) 进程间通信
        作为微内核模块之间协作的基础
            ...
            author: hfcloud(sxysxygm@gmail.com)
              date: 2019.07.31              
 */

#ifndef TAURIX_IPC_H
#define TAURIX_IPC_H
#include <taurix.h>
CSTART

typedef struct tagMessage {
    uint32 message;
    uint32 sender_pid;
    union {
        void *arguments;
        uint32 embed_arguments[4];
    };
    union {
        void *return_vals;
        uint32 embed_return_vals[4];
    };
    uint32 reserved;
}TMessage;

#include <taurix/process.h>

void init_ipc();

//API，触发ipc_send系统调用
int32 ipc_send(TMessage *msg, Process *receiver) EXPORT_SYMBOL(ipc_send);
//API，触发ipc_recv系统调用
int32 ipc_recv(TMessage *msg) EXPORT_SYMBOL(ipc_recv);
//API，触发ipc_post系统调用
int32 ipc_post(TMessage *msg, Process *receiver) EXPORT_SYMBOL(ipc_post);
//API，触发ipc_peek系统调用
int32 ipc_peek(TMessage *msg) EXPORT_SYMBOL(ipc_peek);

//内部使用，ipc相关系统调用的真正实现
int32 ipc_send_impl(TMessage *msg, Process *receiver);
int32 ipc_recv_impl(TMessage *msg);
int32 ipc_post_impl(TMessage *msg, Process *receiver);
int32 ipc_peek_impl(TMessage *msg);

CEND
#endif