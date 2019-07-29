/* File: src/arch/x86_64/main.c
    author: hfcloud(sxysxygm@gmail.com) 
        date: 2019.07.28
*/


#include <taurix.h>

void TaurixCMain() {
    //定义基指针然后用偏移量去访问，编译出来的汇编代码似乎不太对劲。。。
    
    /*
    *(unsigned char *)0xb8000 = 'H';
    *(unsigned char *)0xb8002 = 'e';    
    *(unsigned char *)0xb8004 = 'l';
    *(unsigned char *)0xb8006 = 'l';
    *(unsigned char *)0xb8008 = 'o';
    *(unsigned char *)0xb800a = ' ';
    *(unsigned char *)0xb800c = 'T';
    *(unsigned char *)0xb800e = 'a';
    *(unsigned char *)0xb8010 = 'u';
    *(unsigned char *)0xb8012 = 'r';
    *(unsigned char *)0xb8014 = 'i';
    *(unsigned char *)0xb8016 = 'x';
     */

    ru_text_init();
    char msg[] = "Hello Taurix!";
    ru_text_print(msg);
}
