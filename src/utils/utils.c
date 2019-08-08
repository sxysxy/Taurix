/* File: src/utils/utils.c
    author: hfcloud(sxysxygm@gmail.com) 
        date: 2019.07.28
*/


#include <taurix.h>
#include <taurix/mm/basic_mm.h>

/* ru_dummy
 */
void ru_dummy() {}

//string.h

/* ru_memset
 */
void *ru_memset(void *base, uint8 data, size_t size) {
    for(uint8 *ptr = base; size; size--, ptr++)
        *ptr = data;
    return base;
}

/* ru_memcpy
 */
void *ru_memcpy(void *dest, const void *src, size_t size) {
    uint8 *pdest = (uint8 *)dest, *psrc = (uint8*)src;
    while(size--) 
        *pdest++ = *psrc++;
    return dest;
}

char *ru_strcpy(char *dest, const char *src) {
    char *ori_dest = dest;
    while(*src) *dest++ = *src++;
    *dest = 0;
    return ori_dest;
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

int ru_strncmp(const char *a, const char *b, size_t size) {
    while(size-- && (*a && *b) && *a == *b)
        a++, b++;
    return *a - *b;
}
char* ru_strncpy(char *dest, const char *src, size_t size) {
    char *ori_dest = dest;
    while(size-- && *src) {
        *dest++ = *src++;
    }
    *dest = 0;
    return ori_dest;
}
int ru_memcmp(const void *a, const void *b, size_t size) {
    const char *pa = (const char *)a;
    const char *pb = (const char *)b;
    while(size--) {
        char diff = *pa - *pb;
        if(diff) return diff;
        *pa++, *pb++;
    }
    return 0;
}
char *ru_strcat(char *dest, const char *src) {
    char *ori_dest = dest;

    int len = ru_strlen(dest);
    for(dest += len; *src; ) 
        *dest++ = *src++;
    *dest = 0;
    return ori_dest; 
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