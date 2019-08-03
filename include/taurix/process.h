/* File include/taurix/process.h
        进程
            ...
            author: hfcloud(sxysxygm@gmail.com)
              date: 2019.07.31              
 */

#include <taurix.h>
#ifndef PROCESS_H
#define PROCESS_H

CSTART

struct tagProcess;
//要求Process结构定义中包含下面的ProcessInfo，剩余部分可以将平台相关的数据存入
typedef struct tagProcess Process;  

enum ProcessPrivilege{
  PRIVILEGE_KERNEL = 0,
  PRIVILEGE_USER
};
//-----------进程实现接口
typedef struct tagProcessInfo {

    uint32 pid, parent_id;  //id
    uint32 flags;

    void *entry;            //入口地址，入口为0为无效

    /* 调度相关 */
    uint32 priority;          //优先权 
    uint32 remain_time_slice; //剩余时间片
    uint32 status;           
    void *scheduling_extra;   //调度器可自行扩展的信息

    //栈空间
    void *stack;
    uint32 stack_size;

    //TODO: 补充其它的通用进程信息

    //通用的扩展信息部分
    void *info_extra;
}ProcessInfo;

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
}ProcessScheduler;

//初始化
int32 ps_initialize(ProcessScheduler *ps, uint32 max_process);

//添加进程，失败会返回STATUS_FAILED
int32 ps_add_process(ProcessScheduler *ps, ProcessInfo *info);

//开始进行调度, 使用第一个加入调度器的进程作为第一个进程，duration_per_slice指定了每个时间片的时间（单位：ms）
int32 ps_schedule(ProcessScheduler *ps, uint32 duration_per_slice);

//删除进程还要配合MM模块使用，TODO: 删除进程

//TODO: 补充

CEND

#endif

