/* File: include/taurix/utils.h
    基本的定义
    声明一些公用函数，接口平台无关，如果实现也是平台无关的，大多应在src/utils/下除了main.c之外的源文件中实现
                               如果实现是平台相关的，在src/arch/$arch/arch_utils(n).c(或者汇编语言实现 .S)中实现
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
#define STATUS_SUCCESS 1
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

typedef struct _TObject {
    uint32 typeinfo;
    struct {
        size_t object_lock;
        size_t refer_count;
        void *constructor, *destructor, *query_interface;
    };
}TObject;
#define AS_OBJECT(p) ((TObject*)(p))

//公用工具函数/定义
#ifndef NULL
#ifdef __cplusplus
#define NULL nullptr
#else
#define NULL ((void *)0)
#endif
#endif

#define EXPORT_SYMBOL(x) asm(#x)

//stdarg 兼容
typedef char *va_list;
#define __va_rounded_size(type) (((sizeof(type) + sizeof(int) - 1) / sizeof(int)) * sizeof(int))
#define va_start(ap, last) (ap = ((char*) &(last) + __va_rounded_size(last)))
#define va_arg(ap, type) (ap += __va_rounded_size(type), *((type*) (ap - __va_rounded_size(type))))
#define va_end(ap) (ap = (va_list)0)

//stdio.h
int ru_sprintf_s(char *buf, size_t max_size, const char *format, ...);     //->src/utils/utils.c

//stdlib.h
void *ru_memset(void *base, uint8 data, size_t size) EXPORT_SYMBOL(ru_memset);   //->src/utils/utils.c
void *ru_memcpy(void *dest, const void *src, size_t size) EXPORT_SYMBOL(ru_memcpy);

//string.h
char *ru_strcpy(char *dest, const char *src);       //->src/utils/utils.c
int ru_strlen(const char *str);
int ru_strcmp(const char *a, const char *b);
int ru_strncmp(const char *a, const char *b, size_t size);
char *ru_strncpy(char *dest, const char *src, size_t size);
int ru_memcmp(const void *a, const void *b, size_t size);
char *ru_strcat(char *dest, const char *src);

//math
//整数开根，返回根号x下取整
uint32 ru_isqrt(uint32 x); 

//空函数
void ru_dummy();

//系统控制，下面的函数一般都较为底层，由平台对应的汇编语言实现
//->src/平台/arch_utils.S
//端口IO， 写

void ru_port_write8(uint32 port, uint8 data) EXPORT_SYMBOL(ru_port_write8); 
void ru_port_write16(uint32 port, uint16 data) EXPORT_SYMBOL(ru_port_write16);
void ru_port_write32(uint32 port, uint32 data) EXPORT_SYMBOL(ru_port_write32);

//端口IO, 读
uint8 ru_port_read8(uint32 port) EXPORT_SYMBOL(ru_port_read8);
uint16 ru_port_read16(uint32 port) EXPORT_SYMBOL(ru_port_read16);
uint32 ru_port_read32(uint32 port) EXPORT_SYMBOL(ru_port_read32);

//中断开关控制
void ru_enable_interrupt() EXPORT_SYMBOL(ru_enable_interrupt);   //关中断
void ru_disable_interrupt() EXPORT_SYMBOL(ru_disable_interrupt);  //开中断

//探测一块满足min_size大小的可用的内存 ->src/arch/$arch/arch_utils(n).c
//如果不满足，则会将*baseaddr, *actual_sizeqingling清零
int32 ru_detect_available_memory(size_t min_size, _OUT void **baseaddr, _OUT size_t *actual_size);

//在最开始的文本模式下提供基本的输出信息的功能
int32 ru_text_init();
int32 ru_text_putchar(int ch); //->src/平台/arch_utils(n).c 平台相关的函数
void ru_text_print(const char *text);
void ru_text_set_color(uint32 color);  //设置文本的颜色
void ru_text_set_cursor(int row, int col);
int32 ru_text_get_colmns();        //获得文本模式下文字的 列数（宽度）
int32 ru_text_get_rows();          //获得行数
void ru_text_clear();             //文本模式清屏
//VGA 文本模式的颜色
#define VGA_TEXT_BLUE     1
#define VGA_TEXT_GREEN    2
#define VGA_TEXT_RED      4
#define VGA_TEXT_CYAN    (VGA_TEXT_BLUE + VGA_TEXT_GREEN)
#define VGA_TEXT_PURPLE  (VGA_TEXT_BLUE + VGA_TEXT_RED)
#define VGA_TEXT_YELLOW  (VGA_TEXT_GREEN + VGA_TEXT_RED) 
#define VGA_TEXT_WHITE   (VGA_TEXT_BLUE + VGA_TEXT_GREEN + VGA_TEXT_RED) 

//遇到致命错误时的挂起
void ru_kernel_suspend() EXPORT_SYMBOL(ru_kernel_suspend);  //src->/平台/arch_utils(n).c

//TODO

//->src/utils/utils.c

CEND

#endif