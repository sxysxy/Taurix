/* File: src/mm/basic_mm.h
    提供最基础的内存管理功能，内存分配和释放。
    这个basic内存管理模块只应当用于管理内核保留区内存，在内核建立起完备的内存管理系统之前使用，单线程。
    使用lanzalloc(made by LanzaSchneider(www.lanzainc.xyz))
        author: hfcloud(sxysxygm@gmail.com)
          date: 2019.07.29
 */ 

#include <taurix/mm/basic_mm.h>
#include "lanzalloc.h"

void *basic_mm_malloc(BasicMM *self, size_t size) {
    return lanzalloc_alloc((struct lanzalloc *)self->mm_data, size);
}
void *basic_mm_realloc(BasicMM *self, void *address, size_t size) {
    return lanzalloc_realloc((struct lanzalloc *)self->mm_data, address, size);
}
void basic_mm_free(BasicMM *self, void *ptr) {
    lanzalloc_free((struct lanzalloc *)self->mm_data, ptr);
}
void basic_mm_initialize(BasicMM *self, void *baseptr, size_t size) {
    self->mm_data = lanzalloc_initialize(baseptr, size, ru_isqrt(size));
}
void basic_mm_finalize(BasicMM *self) {
    
}
