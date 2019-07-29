/* File: src/utils/utils.c
    author: hfcloud(sxysxygm@gmail.com) 
        date: 2019.07.28
*/


#include <taurix.h>

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