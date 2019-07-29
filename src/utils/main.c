/* File: src/utils/main.c 
    平台无关的内核主函数
    author: hfcloud(sxysxygm@gmail.com) 
        date: 2019.07.28
*/


#include <taurix.h>

void TaurixCMain() {
    ru_text_init();
    ru_text_print("Welcom to Taurix OS!\n");
    ru_text_print("Now initializing...\n");
}
