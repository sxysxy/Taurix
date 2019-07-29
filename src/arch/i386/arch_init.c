/* File: src/arch/i386/arch_init.c
    目标平台初始化的接口的声明
        author: hfcloud(sxysxygm@gmail.com)

 */

#include <taurix/sys/arch_init.h>

//针对目标平台的设备初始化模块
// 返回 STATUS_SUCCESS 成功
//     STATUS_FAILED
uint32 arch_init() {

}

//初始化内存（为高级内存管理做准备）
uint32 arch_init_memory() {

}

//初始化储存设备
uint32 arch_init_storage() {

}

//初始化时钟
uint32 arch_init_clock() {

}

//初始化键盘输入设备
uint32 arch_init_keyboard() {

}

//初始化图形设备
uint32 arch_init_video() {

}

//杂项的初始化
uint32 arch_init_misc() {
    
}
