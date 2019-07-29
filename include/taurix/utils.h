/* File: include/taurix/utils.h
    基本的定义
    声明一些公用函数
    author: hfcloud(sxysxygm@gmail.com) 
        date: 2019.07.28
*/

#ifndef UTILS_H
#define UTILS_H

#include "taurix.h"

//C
#if defined(__cplusplus)
#define CSTART extern "C" {
#define CEND }
#else 
#define CSTART
#define CEND
#endif

CSTART

//可选的参数修饰说明，表明参数是传入，传出或者传入和传出
#define _IN 
#define _OUT
#define _IN_OUT

//通用返回值
#define STATUS_SUCESS 1
#define STATUS_FAILED 0

//检查数据长度
typedef char __taurix_check_sizeof_char[sizeof(char) == 1 ? 1 : -1];
typedef char __taurix_check_sizeof_short[sizeof(short) == 2 ? 1 : -1];
typedef char __taurix_check_sizeof_int[sizeof(int) == 4 ? 1 : -1];
typedef char __taurix_check_sizeof_longlong[sizeof(long long) == 8 ? 1 : -1];
typedef char __taurix_check_sizeof_voidp[sizeof(void*) == 8 ? 1 : (sizeof(void *) == 4 ? 2 : -1)];

//定义类型别名
typedef char int8;
typedef short int16;
typedef int int32;
typedef long long int64;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned long long uint64;

#ifdef __x86_64__
#define size_t uint64
#elif __i386__
#define size_t uint32
#endif

//公用工具函数
//stdio.h
int ru_sprintf(char *buf, const char *format, ...);     //->src/utils/utils.c

//stdlib.h
void *ru_memset(void *base, uint8 data, size_t size);   //->src/utils/utils.c
void *ru_memcpy(void *dest, void *src, size_t size);

//空函数
void ru_dummy();

//系统控制，下面的函数一般都较为底层，由平台对应的汇编语言实现
//->src/平台/arch_utils.S
//端口IO， 写
void ru_port_write8(uint32 port, uint8 data); 
void ru_port_write16(uint32 port, uint16 data);
//void ru_port_write32(uint32 port, uint32 data);

//端口IO, 读
uint8 ru_port_read8(uint32 port);
uint16 ru_port_read16(uint32 port);
//uint32 ru_port_read32(uint32 port);

//中断开关控制
void ru_enable_interrupt();   //关中断
void ru_disable_interrupt();  //开中断

//在最开始的文本模式下提供基本的输出信息的功能
int32 ru_text_init();
int32 ru_text_putchar(int ch); //->src/平台/arch_utils1.c 平台相关的函数
void ru_text_print(const char *text);
int32 ru_text_printf(const char *format, ...);
void ru_text_set_color(uint32 color);  //设置文本的颜色
void ru_text_set_cursor(int row, int col);
int32 ru_text_get_colmns();        //获得文本模式下文字的 列数（宽度）
int32 ru_text_get_rows();          //获得行数
void ru_text_clear();             //文本模式清屏


//---------------OOP系统--------------
typedef struct tagTObject {
    uint64 flags;  //标记
    void *vptr;    //虚函数表
    struct {
        void (*initialize)(struct tagTObject *self);
        void (*finalize)(struct tagTObject *self);
    }vtbl;
}TObject;
#define TOOP_OBJECT 1

//->src/utils/utils.c
void tobject_initialize(TObject *);
void tobject_finalize(TObject *);

CEND

#endif