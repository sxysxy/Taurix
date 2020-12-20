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
#include <taurix/utils/vector.h>

//内核起始地址
#define KERNEL_BASE_ADDRESS ((void*)0x100000)

//内核保留空间
#define KERNEL_RESERVE_SIZE (32 * 1024 * 1024)

//内核代码保留空间
#define KERNEL_CODE_RESERVE_SIZE (4 * 1024 * 1024)

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

//内核后台进程
void process_kernel_daemon() EXPORT_SYMBOL(process_kernel_daemon);
void process_kernel_daemon() {
    //ru_kernel_suspend();  //never exit
    for(;;);
}

/*
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

    ru_kernel_suspend(); 
}
*/

//暂时的代码，测试ipc
void process_sender() EXPORT_SYMBOL(process_sender);
void process_sender() {
    char sender[] = "Sender starting message\n";
    char data1[] = "Hello world";
    char data2[] = "Hi world";
    char tip1[] = "Sender: Sended data1\n";
    char tip2[] = "Sender: Sended data2\n";
    char tip3[] = "sender: Exited\n";

    /*
    TVector vec;
    vector_init(&vec);
    int a, b;
    vector_push(&vec, 2);
    vector_push(&vec, 3);
    vector_pop(&vec, a);
    vector_pop(&vec, b);

    vector_finalize(&vec);
    char test_vec[30];
    ru_sprintf_s(test_vec, 30, "Sender starting message: %d %d\n", a, b);
    ru_text_print(test_vec);*/
    ru_text_print(sender);

    TMessage msg;
    msg.message = 233;  //message id，测试的时候随便写的
    msg.shared_data = data1;
    msg.sizeof_data = sizeof(data1);
    ru_text_print(tip1);
    ipc_send(&msg, 1);   //在目标进程notify之前阻塞
    msg.message = 233;
    msg.shared_data = data2;
    msg.sizeof_data = sizeof(data2);
    ru_text_print(tip2);
    ipc_send(&msg, 1);
    msg.message = 1024;  //make receiver exit
    msg.sizeof_data = 0;
    ipc_send(&msg, 1);
    ru_text_print(tip3);
    ru_kernel_suspend();
    ps_exit_process(0);
}
void process_receiver() EXPORT_SYMBOL(process_receiver);
void process_receiver() {
    TMessage msg;
    char tip1[] = "Receiver: Got ";
    char tip2[] = "Receiver: Got exiting message\n";
    char recvmsgp[] = "Receiver starting mssage: yep!\n";
    ru_text_print(recvmsgp);

    for(;;) {
        ipc_recv(&msg);  //没有消息时会阻塞
        if(msg.message == 233) {
            ru_text_print(tip1);
            ru_text_print(msg.shared_data);
            ru_text_putchar('\n');
            msg.sizeof_return = 0;
            ipc_notify(&msg);
            continue;
        } else if(msg.message == 1024) {
            ru_text_print(tip2);
            msg.sizeof_return = 0;
            ipc_notify(&msg);
           ru_kernel_suspend();
            ps_exit_process(0);
            continue;
        }
    }
}

void test_ipc(ProcessScheduler *ps) {
    ProcessInfo pinfo;
    ru_memset(&pinfo, 0, sizeof(pinfo));
    //入口点
    pinfo.entry = process_sender;
    pinfo.priority = 40;
    //设置栈
    pinfo.stack = ru_malloc(0x1000);
    pinfo.stack_size = 0x1000;
    //标记
    pinfo.flags = PROCESS_PRESENT | PROCESS_PRIVILEGE_KERNEL;
    ps_add_process(ps, &pinfo);

    pinfo.entry = process_receiver;
    pinfo.priority = 50;    
    pinfo.stack = ru_malloc(0x1000);
    pinfo.stack_size = 0x1000;
    ps_add_process(ps, &pinfo);
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
     
    ru_enable_interrupt();

    //初始化IPC机制
    init_ipc();

    //启动内核后台进程
    ProcessScheduler ps;
    ps_initialize(&ps, 128);

    ProcessInfo pinfo;
    ru_memset(&pinfo, 0, sizeof(pinfo));
    pinfo.entry = process_kernel_daemon;
    pinfo.priority = 5;   //非常低的优先权
    pinfo.stack = ru_malloc(0x500);
    pinfo.stack_size = 0x500;
    pinfo.flags = PROCESS_PRESENT | PROCESS_PRIVILEGE_KERNEL;
    //其它信息会由调度器自动填充
    ps_add_process(&ps, &pinfo);
    
    //加入测试ipc用的进程
    test_ipc(&ps);

    ps_schedule(&ps, 10);  //10ms一个时间片，开始调度
    
    ru_kernel_suspend();   //实际上执行不到这里，前面已经把控制权交给调度器了
}
