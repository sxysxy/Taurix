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

typedef struct tagProcessInfo {
    uint32 pid, parent_id;  //id

    /* 调度相关 */
    uint32 priority;          //优先权 
    uint32 remain_time_slice; //剩余时间片
    void *scheduling_extra;    //调度器可自行扩展的信息
  
    //TODO: 补充其它的通用进程信息

    //通用的扩展信息部分
    void *info_extra;
}ProcessInfo;

int32 process_initialize(Process *proc, ProcessInfo *info);
uint32 process_switch_to(Process *proc);
int32 process_get_info(Process *proc, ProcessInfo *pinfo);

#endif

CEND