/* File: include/taurix/mm/basic_mm.h
    提供最基础的内存管理功能，内存分配和释放。
    这个basic内存管理模块只应当用于管理内核保留区内存，在内核建立起完备的内存管理系统之前使用，单线程。
    使用lanzalloc(made by LanzaSchneider(www.lanzainc.xyz))
        author: hfcloud(sxysxygm@gmail.com)
          date: 2019.07.29
 */ 

#ifndef BASIC_MM_H
#define BASIC_MM_H

#include <taurix.h>

CSTART

typedef struct tagBasicMM {
    void *mm_data;  //当前使用的是lanzalloc,那么它应当指向struct lanzalloc。但这里仍然保留实现无关的接口
}BasicMM;

void basic_mm_initialize(BasicMM *self, void *baseptr, size_t size);
void *basic_mm_malloc(BasicMM *self, size_t size);
void *basic_mm_realloc(BasicMM *self, void *address, size_t size);
void basic_mm_free(BasicMM *self, void *ptr);
void basic_mm_finalize(BasicMM *self);

extern BasicMM ru_basic_mm;
void ru_basic_mm_init(void *baseaddr, size_t size);      //->src/utils/utils.c
void *ru_malloc(size_t size);
void *ru_realloc(void *address, size_t size);
void ru_free(void *ptr);

CEND

#endif
