#ifndef GPT_PART_H
#define GPT_PART_H

#ifdef __cplusplus
extern "C" {
#endif

#include "tfstools.h"

typedef unsigned char GPTGUID[16];

typedef struct tagGPTHeader {
    char                    signature[8];        // "EFI PART"
    unsigned int            version;
    unsigned int            sizeof_header; 
    unsigned int            header_crc32;        //表头的crc32校验  （暂时不用）
    unsigned int            reserved;
    unsigned long long      header_lba;          //主gpt分区表表头lba
    unsigned long long      backup_lba;          //备份gpt分区表表头lba
    unsigned long long      first_partable_lba;  //第一个可以用分区的lba
    unsigned long long      last_partable_lba;   //最后一个可用于分区的lba(备份表lba-1)
    GPTGUID                 disk_guid;         
    unsigned long long      gpt_item_first_lba;  //分区表项第一项的lba(通常是2)
    unsigned int            gpt_part_count;      //分区数量
    unsigned int            gpt_item_size;       //一个分区表项的大小
    unsigned int            gpt_part_crc32;      //分区表项的crc32校验（暂时不用）
    unsigned char           zeros[420];          //zeros
}GPTHeader;

typedef char __check_gpt_header_512_bytes[sizeof(GPTHeader) == 512 ? 1 : -1];

typedef struct tagGPTItem {
    GPTGUID                 type_guid;           //分区类型guid
    GPTGUID                 guid;                //分区guid
    unsigned long long      start_lba;           //起始lba
    unsigned long long      end_lba;             //末尾lba
    unsigned long long      attributes;          //属性
    union {
        char                label[72];           //分区名
        short               label_wchar[36];     //宽字节版
    };
}GPTItem;

typedef char __check_gpt_item_128_bytes[sizeof(GPTItem) == 128 ? 1 : -1];

static const GPTGUID GUID_EMPTY = {0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0};
static const GPTGUID GUID_EFISYSTEM = {0x28, 0x73, 0x2a, 0xc1, 0x1f, 0xf8, 0xd2, 0x11, 0xba, 0x4b, 0x00, 0xa0, 0xc9, 0x3e, 0xc9, 0x3b};
static const GPTGUID GUID_MSRESERVE = {0x16, 0xe3, 0xc9, 0xe3, 0x5c, 0x0b, 0xb8, 0x4d, 0x81, 0x7d, 0xf9, 0x2d, 0xf0, 0x02, 0x15, 0xae};
static const GPTGUID GUID_MSBASICDATA = {0xa2, 0xa0, 0xd0, 0xeb, 0xe5, 0xb9, 0x33, 0x44, 0x87, 0xc0, 0x68, 0xb6, 0xb7, 0x26, 0x99, 0xc7};
//可能一些linux爱好者看到了会生气，可微软数据分区guid和linux数据分区guid真的是一样的呢
#define GUID_LINUXDATA GUID_MSBASICDATA
static const GPTGUID GUID_LINUXSWAP = {0x6d, 0xfd, 0x57, 0x06, 0xab, 0xa4, 0xc4, 0x43, 0x84, 0xe5, 0x09, 0x33, 0xc8, 0x4b, 0x4f, 0x4f};
static const GPTGUID GUID_MBR = {0x41, 0xee, 0x4d, 0x02, 0xe7, 0x33, 0xd3, 0x11, 0x9d, 0x69, 0x00, 0x08, 0xc7, 0x81, 0xf3, 0x9f};
static const GPTGUID GUID_BIOSBOOT = {0x48, 0x61, 0x68, 0x21, 0x49, 0x64, 0x6f, 0x6e, 0x74, 0x4e, 0x65, 0x65, 0x64, 0x45, 0x46, 0x49};
static const GPTGUID GUID_WINDOWS_RECOVERY = {0xa4, 0xbb, 0x94, 0xde, 0xd1, 0x06, 0x40, 0x4d, 0xa1, 0x6a, 0xbf, 0xd5, 0x01, 0x79, 0xd6, 0xac};
static const GPTGUID GUID_APPLE_HFS = {0x00, 0x53, 0x46, 0x48, 0x00, 0x00, 0xaa, 0x11, 0xaa, 0x11, 0x00, 0x30, 0x65, 0x43, 0xec, 0xac};
static const GPTGUID GUID_APPLE_UFS = {0x00, 0x53, 0x46, 0x55, 0x00, 0x00, 0xaa, 0x11, 0xaa, 0x11, 0x00, 0x30, 0x65, 0x43, 0xec, 0xac};

//系统分区
#define PART_ATTRIBUTE_SYSTEM                    (1<<0)

//EFI分区
#define PART_ATTRIBUTE_EFI_HIDE                  (1<<1)

//传统BIOS可引导分区
#define PART_ATTRIBUTE_BIOSBOOT                  (1<<2)

//只读
#define PART_ATTRIBUTE_READONLY                  (1ull<<60)

//隐藏
#define PART_ATTRIBUTE_HIDE                      (1ull<<62)

//不自动挂载
#define PART_ATTRIBUTE_NO_AUTO_MOUNT             (1ull<<63)

#define PART_EMPTY                                0
#define PART_EFISYSTEM                            1
#define PART_MSRESERVE                            2
#define PART_MSBASICDATA                          3
#define PART_LINUXDATA                         PART_MSBASICDATA
#define PART_LINUX_SWAP                           4
#define PART_MBR                                  5
#define PART_BIOSBOOT                             6
#define PART_WINDOWS_RECOVERY                     7
#define PART_APPLE_HFS                            8
#define PART_APPLE_UFS                            9
#define PART_UNKNOWN                              10
static const char *PART_DESCRIPTION[] = {
    "Empty",
    "EFI System",
    "Reserved by Mircosoft",
    "Data",
    "Linux Swapfile",
    "MBR Partition Table",
    "BIOS Boot",
    "Windows Recovery Environment",
    "Apple HFS/HFS+",
    "Apple UFS",
    "Unknown",
};
static const GPTGUID *PART_TYPE_GUID_[] = {
    &GUID_EMPTY,
    &GUID_EFISYSTEM,
    &GUID_MSRESERVE,
    &GUID_MSBASICDATA,
    &GUID_LINUXSWAP,
    &GUID_MBR,
    &GUID_BIOSBOOT,
    &GUID_WINDOWS_RECOVERY,
    &GUID_APPLE_HFS,
    &GUID_APPLE_UFS,
};
#define PART_TYPE_GUID(__i) (*(PART_TYPE_GUID_[__i]))

typedef struct tagGPTPartTool {
    PartTool pt;
    GPTHeader header;
    GPTHeader header_backup;   //备份表
    GPTItem items[128];
    int is_valid_gpt_disk;     //是否为合法的gpt磁盘的标记
    unsigned int num_items;    //header中也有分区数，这里记录的是min(128, header中记录的数量)
}GPTPartTool;

int gpt_part_tool_initialize(GPTPartTool *pt, FSIO io);
int gpt_make_part_table(struct tagPartTool *pt, uint64 disk_size_in_byte);
unsigned int gpt_get_part_count(PartTool *pt);
int gpt_create_part(PartTool *pt, PartInfo *info, FSHandle *target);
int gpt_delete_part(PartTool *pt, FSHandle target);
//int gpt_rename_part(PartTool *pt, FSHandle target, const char *new_name);
FSHandle gpt_enum_part(PartTool *pt, FSHandle last);
int gpt_query_part_info(PartTool *pt, FSHandle target, PartInfo *info);
int gpt_sync_with_disk(struct tagPartTool *pt);

void RandomGUID(GPTGUID *guid);

#ifdef __cplusplus
}
#endif

#endif
