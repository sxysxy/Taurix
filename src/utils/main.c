/* File: src/utils/main.c 
    平台无关的内核主函数
    author: hfcloud(sxysxygm@gmail.com) 
        date: 2019.07.28
*/

#include <taurix.h>
#include <taurix/mm/basic_mm.h>
#include <taurix/arch_init.h>

//内核起始地址
#define KERNEL_BASE_ADDRESS ((void*)0x100000)

//内核保留空间
#define KERNEL_RESERVE_SIZE (32 * 1024 * 1024)

//内核代码保留空间
#define KERNEL_CODE_RESERVE_SIZE (2 * 1024 * 1024)

void hello() {
    ru_text_init();
    ru_text_clear();  //清屏
    ru_text_print("Welcom to Taurix OS! ");
    ru_text_print("Now initializing...\n");
}

int basic_mm_init() {

    //探测找到一块32MB大小的内存给内核使用
    void *mm_base_addr;
    size_t mm_size;
    
    //内存不足
    if(!ru_detect_available_memory(KERNEL_RESERVE_SIZE, &mm_base_addr, &mm_size)) 
        return STATUS_FAILED;

    //内核保留内存不足
    if(KERNEL_BASE_ADDRESS < mm_base_addr || KERNEL_BASE_ADDRESS+KERNEL_RESERVE_SIZE >= mm_base_addr+mm_size) 
        return STATUS_FAILED;

    //内核保留内存从KERNEL_BASE_ADDRESS开始，共32Mb
    ru_basic_mm_init(KERNEL_BASE_ADDRESS+KERNEL_CODE_RESERVE_SIZE, KERNEL_RESERVE_SIZE-KERNEL_CODE_RESERVE_SIZE);

    char *test = ru_malloc(200);
    ru_sprintf_s(test, 200, "[ OK ] Init basic memory management of %dMb Memory\n", KERNEL_RESERVE_SIZE / 1024 / 1024);
    ru_text_set_color(VGA_TEXT_BLUE); //-> TODO: VGA文本模式标准，1是蓝色。。。先暂时这样，以后图形驱动搞好了这里需要改一下
    ru_text_print(test);  
    ru_free(test);
    return STATUS_SUCCESS;
}

void TaurixCMain() {
    hello();

    //初始化基本内存管理
    if(!basic_mm_init()) {
        char tmp[100];
        ru_text_set_color(VGA_TEXT_RED);
        ru_sprintf_s(tmp, 100, "[ FAILED ] Failed to find at least %dMb available memory for kernel, halt.\n", KERNEL_RESERVE_SIZE / 1024 / 1024);
        ru_text_print(tmp);
        ru_kernel_suspend();
    }

    //初始化目标平台
    if(!arch_init()) {
        ru_text_set_color(VGA_TEXT_RED);
        ru_text_print("[ FAILED ] Failed to initialize the architecture, halt.\n");
        ru_kernel_suspend();
    } else {
        ru_text_set_color(VGA_TEXT_BLUE);
        ru_text_print("[ OK ] Initialize the architecture, TODO: Finish this module.\n");
    }
}
