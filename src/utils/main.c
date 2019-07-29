/* File: src/utils/main.c 
    平台无关的内核主函数
    author: hfcloud(sxysxygm@gmail.com) 
        date: 2019.07.28
*/

#include <taurix.h>
#include <taurix/mm/basic_mm.h>

void hello() {
    ru_text_init();
    ru_text_clear();  //清屏
    ru_text_print("Welcom to Taurix OS!\n");
    ru_text_print("Now initializing...\n");
}

int basic_mm_init() {
    ru_basic_mm_init();

    //TODO: 加上对已经占用的内存的申请（例如内核，栈使用的空间）  !重要

    char *test = ru_malloc(100);
    ru_strcpy(test, " [ OK ] Init basic memory management of lowest 32Mb memory\n");
    ru_text_set_color(1); //-> TODO: VGA文本模式标准，1是蓝色。。。先暂时这样，以后图形驱动搞好了这里需要改一下
    ru_text_print(test);  
    ru_free(test);
    return STATUS_SUCCESS;
}

void TaurixCMain() {
    hello();

    basic_mm_init();

}
