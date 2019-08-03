/* File: src/utils/main.c 
    平台无关的内核主函数
    author: hfcloud(sxysxygm@gmail.com) 
        date: 2019.07.28
*/

#include <taurix.h>
#include <taurix/mm/basic_mm.h>
#include <taurix/arch_init.h>
#include <taurix/process.h>

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
    ru_text_set_color(VGA_TEXT_BLUE); 
    ru_text_print(test);  
    ru_free(test);
    return STATUS_SUCCESS;
}

//暂时的代码，测试多任务
#include <taurix/arch/i386/i386_utils.h>
void test_long_jmp(void) {
    ru_text_print("[ OK ] Long jump ok\n");
    ru_kernel_suspend();
}

int process1_main() {
    //ru_text_print("Process 1: Hello world\n");
    for(;;); //这家伙在用户态也调用不了suspend省电一点地挂起，也没有实现进程sleep系统调用，就先这样吧。
}

void test_process() {
    ProcessInfo pinfo;
    pinfo.entry = process1_main;
    pinfo.pid = 1, pinfo.parent_id = 0;
    pinfo.priority = 20;
    pinfo.stack = ru_malloc(0x10000);
    pinfo.stack_size = 0x10000;
    Process *proc1 = process_initialize(&pinfo);
    process_switch_to(proc1);
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
    
    //TODO: fixme: process 
    ru_enable_interrupt();
    //i386_jmp_sel(1, test_long_jmp);  //OK
    test_process();
    ru_kernel_suspend();
}
