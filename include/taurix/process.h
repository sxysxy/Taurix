/* File include/taurix/process.h
        进程
            ...
            author: hfcloud(sxysxygm@gmail.com)
              date: 2019.07.31              
 */

#include <taurix.h>
#ifndef PROCESS_H
#define PROCESS_H

#include <taurix/timer.h>

CSTART

struct tagProcess;
//要求Process结构定义中包含下面的ProcessInfo，剩余部分可以将平台相关的数据存入
typedef struct tagProcess Process;  

struct tagMessage;

//进程IPC Queuer队列
typedef struct tagProcessQueuer {
    //发送这个queuer的进程id
    uint32 process_id;
    //struct tagMessage *message;
    struct tagProcessQueuer *next;
}ProcessQueuer;

enum ProcessPrivilege{
  PRIVILEGE_KERNEL = 0,
  PRIVILEGE_USER
};

struct tagProcessScheduler;
//-----------进程实现接口
typedef struct tagProcessInfo {

    uint32 pid, parent_id;  //id

    uint16 status;
    uint16 flags;

    void *entry;            //入口地址，入口为0为无效

    /* 调度相关 */
    uint32 priority;          //优先权 
    uint32 remain_time_slice; //剩余时间片         
    void *scheduling_extra;   //调度器可自行扩展的信息

    //栈空间
    void *stack;
    uint32 stack_size;

    //退出码
    uint32 exit_code;

    //调度器
    struct tagProcessScheduler *scheduler;

    //同步ipc 数据部分
    struct tagMessage *queuing_message;  //当前消息
    ProcessQueuer *queuing_list;
    ProcessQueuer *queuing_tail;

    //TODO: 补充其它的通用进程信息

    //通用的扩展信息部分
    void *info_extra;
}ProcessInfo;

#define PROCESS_STATUS_READY        0
#define PROCESS_STATUS_RUNNING      1
#define PROCESS_STATUS_RUNNABLE     PROCESS_STATUS_RUNNING
#define PROCESS_STATUS_RECEIVING    2
#define PROCESS_STATUS_SENDING      3
#define PROCESS_STATUS_DEADLOCKED   4
#define PROCESS_STATUS_ZOMBIE       5
#define PROCESS_STATUS_DEAD         6

//由于Process是不完全类型，这里提供接口查询一个Process结构占用的内存空间的大小（in bytes)
size_t process_query_sizeof_process();
int32 process_initialize(Process *proc, ProcessInfo *info);
int32 process_switch_to(Process *target, uint32 flags);       //flags见后文
//上面两个函数只能由在内核态的进程调度器主动调用，而不应由进程自己调用（可能会产生逻辑上的错误）
int32 process_get_info(Process *proc, ProcessInfo *pinfo);

//有效的进程的标记
#define PROCESS_PRESENT   0x01    

//进程特权级：内核级（默认）
#define PROCESS_PRIVILEGE_KERNEL    0x02

//进程特权级：用户级
#define PROCESS_PRIVILEGE_USER      0x04

//process_switch_to 由时钟中断引发调度后引发切换
#define PROCESS_SCHEDULE_FROM_INT        0x01

//process_switch_to 由软件请求进程调度器引发切换
#define PROCESS_SCHEDULE_FROM_REQ        0x02

//-----------进程调度接口
typedef struct tagProcessScheduler {
    Process *proc_table;
    uint32 max_process;
    Process *current;
    uint32 flags;
}ProcessScheduler;

#define PROCESS_SCHEDULER_LOCKED        0x01


//初始化
int32 ps_initialize(ProcessScheduler *ps, uint32 max_process);

//添加进程，失败会返回STATUS_FAILED
int32 ps_add_process(ProcessScheduler *ps, ProcessInfo *info);

//获得进程
int32 ps_get_process(ProcessScheduler *ps, _IN uint32 pid, _OUT Process **process);

//阻塞进程
int32 ps_block_process(ProcessScheduler *ps, uint32 pid, uint32 new_status);

//解除进程阻塞
int32 ps_unblock_process(ProcessScheduler *ps, uint32 pid);

//死锁检查
int32 ps_check_deadlock(ProcessScheduler *ps, uint32 pid);

//由进程调用，退出进程
void ps_exit_process(uint32 exit_code);

//开始进行调度, 使用第一个加入调度器的进程作为第一个进程，duration_per_slice指定了每个时间片的时间（单位：ms）
int32 ps_schedule(ProcessScheduler *ps, uint32 duration_per_slice);

//获得正在工作的调度器
ProcessScheduler *ps_get_working_scheduler() EXPORT_SYMBOL(ps_get_working_scheduler);

//引发当前工作中调度器立刻重新调度
void ps_immdiate_reschedule(void *reserved);

//删除进程还要配合MM模块使用，TODO: 删除进程

//TODO: 补充

CEND

#endif

