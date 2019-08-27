/* File: src/utils/main.c 
    平台无关的内核主函数
    author: hfcloud(sxysxygm@gmail.com) 
        date: 2019.07.28
*/

#include <taurix.h>
#include <taurix/mm/basic_mm.h>
#include <taurix/arch_init.h>
#include <taurix/process.h>
#include <taurix/ipc.h>

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
int process1_main() EXPORT_SYMBOL(process1_main);
int process2_main() EXPORT_SYMBOL(process2_main);
int process3_main() EXPORT_SYMBOL(process3_main);

int process1_main() {
    int cnt = 0;
    for(;;) {
        cnt++;
        if(cnt == 1000000) {
            ru_text_print("A");
            cnt = 0;
        }
    }
}

int process2_main() {
    int cnt = 0;
    for(;;) {
        cnt++;
        if(cnt == 1000000) {
            ru_text_print("B");
            cnt = 0;
        }
    }
}

int process3_main() {
    int cnt = 0;
    for(;;) {
        cnt++;
        if(cnt == 1000000) {
            ru_text_print("C");
            cnt = 0;
        }
    }
}

void test_process() {
    ProcessScheduler ps;
    ps_initialize(&ps, 3);

    ProcessInfo pinfo;
    ru_memset(&pinfo, 0, sizeof(pinfo));
    //入口点
    pinfo.entry = process1_main;
    pinfo.pid = 1, pinfo.parent_id = 0;
    pinfo.priority = 40;
    //设置栈
    pinfo.stack = ru_malloc(0x500);
    pinfo.stack_size = 0x500;
    //标记
    pinfo.flags = PROCESS_PRESENT | PROCESS_PRIVILEGE_KERNEL;
    ps_add_process(&ps, &pinfo);

    pinfo.entry = process2_main;
    pinfo.pid = 2, pinfo.parent_id = 0;
    pinfo.priority = 30;    //是第一个进程的权重的2倍
    pinfo.stack = ru_malloc(0x500);
    pinfo.stack_size = 0x500;
    ps_add_process(&ps, &pinfo);

    pinfo.entry = process3_main;
    pinfo.pid = 3, pinfo.parent_id = 0;
    pinfo.priority = 20;   
    pinfo.stack = ru_malloc(0x500);
    pinfo.stack_size = 0x500;
    ps_add_process(&ps, &pinfo);

    ps_schedule(&ps, 10);  //10ms一个时间片，开始调度
    ru_kernel_suspend(); 
}


//暂时的代码，测试ipc
void process_sender() EXPORT_SYMBOL(process_sender);
void process_sender() {
    char data1[] = "Hello world";
    char data2[] = "Hi world";

    TMessage msg;
    msg.message = 233;  //message id，测试的时候随便写的
    msg.shared_data = data1;
    msg.sizeof_data = sizeof(data1);
    ipc_send(&msg, 1);   //在目标进程notify之前阻塞
    ru_text_print("sender: Sended data1\n");
    msg.shared_data = data2;
    msg.sizeof_data = sizeof(data2);
    ipc_send(&msg, 1);
    ru_text_print("sender: Sended data2\n");
    msg.message = 1024;  //make receiver exit
    msg.sizeof_data = 0;
    ipc_send(&msg, 1);
    ps_exit_process(0);
}
void process_receiver() EXPORT_SYMBOL(process_receiver);
void process_receiver() {
    TMessage msg;
    while(1) {
        ipc_recv(&msg);  //没有消息时会阻塞
        if(msg.message == 233) {
            ru_text_print("receiver: Got ");
            ru_text_print(msg.shared_data);
            ru_text_putchar('\n');
            msg.sizeof_return = 0;
            ipc_notify(&msg);
        } else if(msg.message == 1024) {
            ru_text_print("receiver: Exited");
            msg.sizeof_return = 0;
            ipc_notify(&msg);
            ps_exit_process(0);
        }
    }
}

void test_ipc() {
    ProcessScheduler ps;
    ps_initialize(&ps, 2);

    ProcessInfo pinfo;
    ru_memset(&pinfo, 0, sizeof(pinfo));
    //入口点
    pinfo.entry = process_sender;
    pinfo.priority = 40;
    //设置栈
    pinfo.stack = ru_malloc(0x500);
    pinfo.stack_size = 0x500;
    //标记
    pinfo.flags = PROCESS_PRESENT | PROCESS_PRIVILEGE_KERNEL;
    ps_add_process(&ps, &pinfo);

    pinfo.entry = process_receiver;
    pinfo.priority = 50;    
    pinfo.stack = ru_malloc(0x500);
    pinfo.stack_size = 0x500;
    ps_add_process(&ps, &pinfo);

    ps_schedule(&ps, 10);

    ru_kernel_suspend();
}

void TaurixCMain() EXPORT_SYMBOL(TaurixCMain);
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
    
    init_ipc();

    //TODO: fixme: process 
    ru_enable_interrupt();
    //test_process();
    
    test_ipc();
    
    ru_kernel_suspend();
}
