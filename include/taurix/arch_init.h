/* File: include/taurix/arch_init.h
    平台无关的 目标平台初始化的接口的声明
    接口的实现应当在 src/arch/$arch/arch_init.c
        author: hfcloud(sxysxygm@gmail.com)
          date: 2019.07.30
 */

#ifndef ARCH_INIT_H
#define ARCH_INIT_H

#include <taurix.h>

CSTART

//针对目标平台的设备初始化模块
// 返回 STATUS_SUCCESS 成功
//     STATUS_FAILED
uint32 arch_init();

//初始化内存（为高级内存管理做准备）
uint32 arch_init_memory();

//初始化储存设备
uint32 arch_init_storage();

//初始化时钟
uint32 arch_init_clock();

//初始化键盘输入设备
uint32 arch_init_keyboard();

//初始化图形设备
uint32 arch_init_video();

//杂项的初始化
uint32 arch_init_misc();

#endif

CEND