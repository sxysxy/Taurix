// 注意，暂时不支持长文件名


#ifndef FAT32_H
#define FAT32_H

#ifdef __cplusplus
extern "C" {
#endif 

#include "tfstools.h"

#ifdef _MSC_VER
#pragma pack(push, 1)
struct tagFAT32Header {
#else 
struct __attribute__((packed)) tagFAT32Header {
#endif
    uint8           jmp[3];                 //0x00 - 0x02 跳转指令
    uint8           OEM[8];                 //0x03 - 0x0a OEM，工具创建Fat32文件系统时会写入"tfstool\0"
    uint16          bytes_per_sector;       //0x0b - 0x0c 每个lba(sector)字节数，通常都是512，即便一些设备的单个扇区大小实际上不是512，也会向软件层兼容512
    uint8           sectors_per_clust;      //0x0d        每个簇的的扇区数
    uint16          num_reserved;           //0x0e - 0x0f 第一个FAT表前面的保留扇区的数目
    uint8           num_fat;                //0x10        FAT个数（一般来说FAT32是2个）
    uint16          num_root_entries;       //0x11 - 0x12 根目录条目个数最大值，FAT32不使用，为0
    uint16          num_sectors16;          //0x13 - 0x14 总扇区数（如果是0使用num_sectors, tfstools总会使用num_sectors
    uint8           material;               //0x15        介质描述,由于使用lba寻址这里实际上没什么用，固定0xF8
    uint16          sectors_per_fat16;      //0x16 - 0x17 每个FAT的扇区数(for FAT16.FAT32不使用)
    uint32          zeros0;                 //0x18 - 0x1b C/H/S模式使用的信息，无用
    uint32          sectors_hidden;         //0x1c - 0x1f 隐藏扇区
    uint32          num_sectors;            //0x20 - 0x23 总扇区数
    uint32          sectors_per_fat32;      //0x24 - 0x27 每个FAT扇区数(for FAT32)
    uint16          flags;                  //0x28 - 0x29 flags
    uint16          version;                //0x2a - 0x2b 版本号
    uint32          root_dir_start;         //0x2c - 0x2f 根目录起始簇号，一般为2 
    uint16          fsinfo;                 //0x30 - 0x31 fsinfo扇区
    uint16          backup;                 //0x32 - 0x33 引导扇区备份
    uint8           reserved[12];           //0x34 - 0x3f FAT32保留
    uint8           device_code;            //0x40        设备代号，tfstools将使用0x80(主硬盘)
    uint8           zeros4;                 //0x41        未使用
    uint8           signature;              //0x42        标记?
    uint32          partition_index;        //0x43 - 0x46 卷序号
    uint8           partition_label[11];    //0x47 - 0x51 卷标
    uint8           fat32[8];               //0x52 - 0x59 "FAT32"
    uint8           code[420];              //0x5a -0x1fd 引导代码 
    uint16          aa55;                   //0x1fe-0x1ff 0xaa55           
};
#ifdef _MSC_VER
#pragma pack(pop)
#endif

typedef struct tagFAT32Header FAT32Header;

typedef char __tfstools_check_fat32header_512_bytes[sizeof(FAT32Header) == 512 ? 1:-1];

//FAT32的FSInfo信息
#ifdef _MSC_VER
#pragma pack(push, 1)
struct tagFAT32Info {
#else
struct __attribute__((packed)) tagFAT32Info {
#endif
    unsigned int          x41615252;         //固定的0x41615252
    char                  unused[480];       //未使用
    unsigned int          x61417272;         //固定的0x61417272
    unsigned int          free_clusters;     //空闲簇的数量
    unsigned int          next_free_cluster; //下一个空闲簇的簇号
    char                  unused2[14];        //未使用
    unsigned short        aa55;              //结束标志，固定的0xaa55;
};
#ifdef _MSC_VER
#pragma pack(pop)
#endif
typedef char __tfstools_check_fat32info_512bytes[sizeof(struct tagFAT32Info) == 512 ? 1 : -1];
typedef struct tagFAT32Info FAT32Info;

typedef struct tagFAT32Tool {
    FSTool fs;
    //fat相关：
    uint64 fat_lba;             //主FAT表的位置
    uint32 num_fat;             //个数
    uint32 sectors_per_fat;     //每个fat表占的lba数
    uint32 sectors_per_cluster; //每个簇占用的lba数
    uint32 root_dir_cluster;
    int is_valid_fat32;

    //注意！！！如果在一个函数里修改了buffer的内容，调用其它可能会修改buffer内容的函数时要特别注意！！！
    char cluster_buffer[8 * 512];    //临时使用的簇缓冲区，不支持大于8kb的簇     
    char sector_buffer[512];
    FAT32Info fsinfo;
}FAT32Tool;

int fat32_fstool_initialize(FAT32Tool *fat, FSIO io, uint64 start_lba, uint64 end_lba);
int fat32_makefs(struct tagFSTool *fs);                                                       //创建无文件的文件系统
int fat32_create_file(struct tagFSTool *fs, FSHandle dir, FileInfo *info, FSHandle *handle);  //创建文件
int fat32_delete_file(struct tagFSTool *fs, FSHandle target);                                 //删除文件
FSHandle fat32_enum_dir(struct tagFSTool *fs, FSHandle dir, FSHandle last, FileInfo *info);   //列举目录dir内文件(包括子目录)，第一次last为NULL

int fat32_query_file_info(struct tagFSTool *fs, FSHandle file, FileInfo *info);         //查询文件信息

uint64 fat32_fread(struct tagFSTool *fs, FSHandle file, const void *buffer, uint64 buffer_size, FSHandle *offset_handle);
uint64 fat32_fwrite(struct tagFSTool *fs, FSHandle file, const void *data, uint64 data_size, FSHandle *offset_handle);
    
int fat32_fseek(struct tagFSTool *fs, FSHandle file, uint64 offset, FSHandle *offset_handle);

int fat32_sync_information(struct tagFSTool *fs);

#define FAT32_FREE_CLUSTER                  0
#define FAT32_RESERVED_CLUSTER              1
#define FAT32_RESERVED_CLUSTER_ID_START     0x0ffffff0
#define FAT32_RESERVED_CLUSTER_ID_END       0x0ffffff6
#define FAT32_BAD_CLUSTER                   0x0ffffff7
#define FAT32_FILE_END                      0x0fffffff
#define FAT32_IS_END_CLUSTAR(c)             (c >= 0x0ffffff8 && c <= 0x0fffffff)

//FAT32目录表表项结构
#ifdef _MSC_VER
#pragma pack(push, 1)
struct tagFAT32FileItem {
#else 
struct __attribute__((packed)) tagFAT32FileItem {
#endif
                            //0x00-0x07 dos文件名
                            //dosname第一个字节的特殊值：
                            //0x00  这个条目有用并且后面没有被占用的条目（wiki上是这么说的。。。）
                            //0x05  转义为字符"\xe5"，与下面的0xe5不同
                            //0x2e  . 或者.. (指向本目录自己或上一级目录)  
                            //0xe5  已被删除。将这个标记替换掉可能能够恢复文件
    unsigned char         dosname[8];                
                                                              
    char                  dosname_ext[3];            //0x08-0x0a dos文件名扩展名部分
                            
                            //   0x0b 属性:
                            //   mask     含义
                            //  (1<<0)    只读
                            //  (1<<1)    隐藏
                            //  (1<<2)    系统
                            //  (1<<3)    卷标
                            //  (1<<4)   子目录项
                            //  (1<<5)    文件
                            //  (1<<6)    设备
                            //  (1<<7)   未使用
                            //如果attribute值为0x0f表示是长文件条目
    unsigned char         attribute;                               
    unsigned char         reserved_for_NT;    //0x0c NT保留?
    unsigned char         create_time_in2s;   //0x0d 创建时间的2s部分，单位为10ms，有意义的数值从0-199。这个设计真的很智障
    
                            //0x0e - 0x0f 创建时间的时分秒，秒数*2+偏移0x0d处算出的秒数才是实际的时间
                            // bits 15-11 小时
                            // bits 10-5  分钟
                            // bits 4-0   秒/2
    unsigned short        create_time_hmsdiv2;

                            //0x10 - 0x11 创建时间的年月日
                            // bits 15-9  年，最大值为127，这里的数值加上1980为实际的年份
                            // bits  8-5  月，这里的数值就是月份
                            // bits  4-0  日 (1-31)
    unsigned short        create_time_ymd;        


    unsigned short        access_time_ymd;     //0x12 - 0x13 最近访问日期，格式同create_time_ymd 
    unsigned short        first_cluster_hight; //0x14 - 0x15 文件内容的第一个簇的编号的高两字节
    unsigned short        modify_time_hmsdiv2; //0x16 - 0x17 最后修改的时间(时分秒)，格式同create_time_hmsdiv2
    unsigned short        modify_time_ymd;     //0x18 - 0x19 最后修改时间的年月日
    unsigned short        first_cluster_low;   //0x1a - 0x1b 文件内容的第一个簇的编号的低两字节
    unsigned int          file_size_in_byte;   //0x1c - 0x1f 文件大小，字节数       
};
#ifdef _MSC_VER
#pragma pack(pop)
#endif

typedef struct tagFAT32FileItem FAT32FileItem;


#ifdef __cplusplus
}
#endif 

#endif