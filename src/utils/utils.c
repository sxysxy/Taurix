/* File: src/utils/utils.c
    author: hfcloud(sxysxygm@gmail.com) 
        date: 2019.07.28
*/


#include <taurix.h>
#include <taurix/mm/basic_mm.h>

/* ru_dummy
 */
void ru_dummy() {}

/* ru_sprintf
 */
int ru_sprintf(char *buf, const char *format, ...) {
    //TODO: 
    return 0;
}

/* ru_memset
 */
void *ru_memset(void *base, uint8 data, size_t size) {
    for(uint8 *ptr = base; size; size--, ptr++)
        *ptr = data;
    return base;
}

/* ru_memcpy
 */
void *ru_memcpy(void *dest, void *src, size_t size) {
    uint8 *pdest = (uint8 *)dest, *psrc = (uint8*)src;
    while(size--) 
        *pdest++ = *psrc++;
    return dest;
}

//string.h
char *ru_strcpy(char *dest, const char *src) {
    while(*src) *dest++ = *src++;
    *dest = 0;
    return dest;
}

int ru_strlen(const char *str) {
    const char *p = str;
    while(*p)p++;
    return p-str;
}

int ru_strcmp(const char *a, const char *b) {
    while((*a && *b) && *a == *b)
        a++, b++;
    return *a - *b; 
}

/* ru_isqrt
 */
uint32 ru_isqrt(uint32 x) {
    uint32 l = 0, r = x < 65535 ? x : 65535, ans = 0;
    while(l <= r) {
        uint32 m = (l+r) >> 1;
        if(m * m <= x) {
            ans = m;
            l = m+1;
        }else {
            r = m-1;
        }
    }
    return ans;
} 

//basic_mm
//基本内存管理器，定义
BasicMM ru_basic_mm;
void ru_basic_mm_init(void *baseaddr, size_t size) {
    basic_mm_initialize(&ru_basic_mm, baseaddr, size);
}
void *ru_malloc(size_t size) {
    return basic_mm_malloc(&ru_basic_mm, size);
}
void *ru_realloc(void *address, size_t size) {
    return basic_mm_realloc(&ru_basic_mm, address, size);
}
void ru_free(void *ptr) {
    basic_mm_free(&ru_basic_mm, ptr);
}

//OOP system 
void tobject_initialize(TObject *self) {
    self->flags = 0;
}

void tobject_finalize(TObject *this) { }