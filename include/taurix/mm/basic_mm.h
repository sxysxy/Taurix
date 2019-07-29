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
    TObject basic;
    struct VTable {
        void*(*malloc) (size_t size);
        void (*free) (void *ptr);
    }vtbl;
}BasicMM;

void basic_mm_initialize(BasicMM *self, void *baseptr, size_t size);
void basic_mm_malloc(BasicMM *self, size_t size);
void basic_mm_free(BasicMM *self, void *ptr);
void basic_mm_finalize(BasicMM *self);

CEND

#endif
