#ifndef TFSTOOLS_H
#define TFSTOOLS_H

#ifdef __cplusplus
extern "C" {
#endif

//Taurix 补丁：
#include "taurix_fs_patch.h"
//#define STATUS_SUCCESS                 0
//#define STATUS_FAILED                  1

#define ERROR_TFS_START                   1000
//不合法的磁盘镜像
#define ERROR_TFS_INVALID_DISK            (ERROR_TFS_START+1)
//没有足够空间
#define ERROR_TFS_NO_SPACE                (ERROR_TFS_START+2)
//位置冲突
#define ERROR_TFS_CONFLICT                (ERROR_TFS_START+3)
//非法参数
#define ERROR_TFS_INVALID_ARGUMENT        (ERROR_TFS_START+4)
//分区/文件/目录 不存在  <大多针对删除操作>
#define ERROR_TFS_NOT_EXIST               (ERROR_TFS_START+5)
//分区/文件/目录 已存在  <大多针对创建操作>
#define ERROR_TFS_ALREADY_EXIST           (ERROR_TFS_START+6) 
//功能未实现
#define ERROR_TFS_NO_IMPLEMENT            (ERROR_TFS_START+7)
//非法句柄
#define ERROR_TFS_INVALID_HANDLE          (ERROR_TFS_START+8)
//非法文件系统
#define ERROR_TFS_INVALID_FS              (ERROR_TFS_START+9)
//非法文件名
#define ERROR_TFS_INVALID_FILENAME        (ERROR_TFS_START+10)
//不支持的特性
#define ERROR_TFS_UNSUPPORTED_FEATURE     (ERROR_TFS_START+16)

static const char *ERROR_TFS_STRING[] = {0, "Invalid disk", 
                                            "No enough space", 
                                            "Conflict", 
                                            "Invalid argument",
                                            "Not exist",
                                            "Already exist",
                                            "Not implemented",
                                            "Invalid handle",
                                            "Invalid filesystem",
                                            "Invalid filename",
                                            "Unsupported feature"};

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned long long uint64;

typedef uint64 FSHandle;
typedef FSHandle FileHandle;
typedef FSHandle OffsetHandle;

typedef struct tagFSIO {
    uint64 (*read_lba)(uint64 start, uint64 end, void *buf);
    uint64 (*write_lba)(uint64 start, uint64 end, void *buf);
}FSIO;

//分区根据以0作为无效句柄的标识
//分区工具接口
//暂时钦点GPT分区表了所以。。
typedef struct tagPartInfo {
    char label[36];  /* 标签 */
    int type;        //分区类型，GPT分区表可以根据分区guid得到
    
    unsigned long long attribute;
    uint64 start_lba, end_lba;

    unsigned char guid[16];  //guid 
    unsigned char type_guid[16];
}PartInfo;

typedef struct tagPartTool {
    FSIO io;
    int (*make_part_table)(struct tagPartTool *pt, uint64 disk_size_in_byte);            //将磁盘镜像前size_in_bytes做成GPT磁盘
    unsigned int (*get_part_count)(struct tagPartTool *pt);                              //获得分区数（对于比如gpt分区表是固定的128），实际上没有意义的分区类型guid为空
    int (*create_part)(struct tagPartTool *pt, PartInfo *info, FSHandle *target);        //(创建分区)
    int (*delete_part)(struct tagPartTool *pt, FSHandle target);                         //删除分区
    FSHandle (*enum_part) (struct tagPartTool *pt, FSHandle last);                       //枚举所有分区，第一次last为NULL
    int (*query_part_info)(struct tagPartTool *pt, FSHandle target, PartInfo *info);     //查询分区信息
    int (*sync_with_disk) (struct tagPartTool *pt);                                      //将分区表写入磁盘
}PartTool;

//文件系统接口
typedef struct tagFileInfo {
    union {   //文件全名（包括扩展名等）
        char name_char[512];
        short name_wchar[256];
    };
    uint64 attributes;       //属性
    int is_directory;        //是否为目录的标记

    //创建时间的年月日时分秒。
    union {
        struct {
            short created_year;   //16bits
            uint8 created_month;  //8bits
            uint8 created_day;    //8bits
            uint8 created_hour;   //8bits
            uint8 created_minute; //8bits
            uint8 created_second; //8bits
        };
        uint64 created_ymdhms;   
    };

    //访问时间的年月日时分秒。
    union {
        struct {
            short access_year;   //16bits
            uint8 access_month;  //8bits
            uint8 access_day;    //8bits
            uint8 access_hour;   //8bits
            uint8 access_minute; //8bits
            uint8 access_second; //8bits
        };
        uint64 access_ymdhms;   
    };

    //修改时间的年月日时分秒。
    union {
        struct {
            short modify_year;   //16bits
            uint8 modify_month;  //8bits
            uint8 modify_day;    //8bits
            uint8 modify_hour;   //8bits
            uint8 modify_minute; //8bits
            uint8 modify_second; //8bits
        };
        uint64 modify_ymdhms;   
    };
    
    uint64 file_size_in_bytes;      //文件大小(bytes)
    uint64 file_occupies_in_bytes;  //占用储存介质的字节数
}FileInfo;

//普通文件
#define FILE_ATTRIBUTE_NORMAL              0
//只读
#define FILE_ATTRIBUTE_READONLY            (1<<0)
//隐藏
#define FILE_ATTRIBUTE_HIDDEN              (1<<1)
//设备文件
#define FILE_ATTRIBUTE_DEVICE              (1<<2)
//符号链接(软链接)
#define FILE_ATTRIBUTE_SYMBOL_LINK         (1<<3)
//系统文件
#define FILE_ATTRIBUTE_SYSTEM              (1<<4)

//小端序
#define YMDHMS(_Y,_M,_D,_H,_MI,_S) ((_Y) | ((_M) << 16) | ((_D) << 24) | ((_H) << 32) | ((_MI) << 40) | ((_S) << 48))

//文件系统以-1(0xffffffffffffffffull) 作为无效句柄
#define FSTOOL_INVALID_HANDLE    0xffffffffffffffffull
typedef struct tagFSTool {
    FSIO io;
    uint64 start_lba, end_lba;                                                           //起始和终止lba，可以从分区工具得到
    int (*makefs)(struct tagFSTool *fs);                                                 //创建无文件的文件系统
    
    //新建/打开文件，取决于mode
    //dir为目录文件句柄，如果为FSTOOL_INVALID_HANDLE则使用根目录
    //handle为传出参数，用于接收文件句柄。可以为空指针（不接收）
    //返回值为错误代码（0为成功）
    int (*create_file)(struct tagFSTool *fs, FileHandle dir, FileInfo *info, FileHandle *handle);
    
    int (*delete_file)(struct tagFSTool *fs, FileHandle target);                           //删除文件

    //列举目录dir内文件(包括子目录)，dir参数为FSTOOL_INVALID_HANDLE使用根目录，第一次last为FSTOOL_INVALID_HANDLE，从目录中第一个文件开始
    //如果info参数不为空指针，将会同时通过info参数返回函数所返回的文件句柄指向的文件的信息，这样做将遍历目录与查询目录下文件信息一起做了。
    //目的是为了尽量减少磁盘IO次数。
    FileHandle (*enum_dir)(struct tagFSTool *fs, FileHandle dir, FileHandle last, FileInfo *info);         

    //查询单个文件的信息，file为文件句柄，*info为接收信息的结构体
    int (*query_file_info)(struct tagFSTool *fs, FileHandle file, FileInfo *info);

    //对非目录文件的读/写，模式相当于C语言的二进制模式
    //file为文件句柄，不可以是非法句柄（file传入FSTOOL_INVALID_HANDLE不表示根目录，是非法参数，不允许直接对目录文件本身读写）
    //offset_handle既是传入参数又是传出参数，不可以为空指针，当文件不能一次读/写完的时候，offset_handle用于记录文件指针的偏移量。
    //*offset_handle为0时，被认为是读/写的偏移地址是文件开头，读/写后会改变*offset_handle
    //返回值为实际读/写的字节数
    uint64 (*fread)(struct tagFSTool *fs, FSHandle file, const void *buffer, uint64 buffer_size, FSHandle *offset_handle);
    uint64 (*fwrite)(struct tagFSTool *fs, FSHandle file, const void *data, uint64 data_size, FSHandle *offset_handle);
    
    //得到offset对应的offset_handle
    //注意，不保证offset == *offset_handle，*offset_handle是跟具体文件系统实现有关的数值，而非实现无关的偏移量
    int (*fseek)(struct tagFSTool *fs, FSHandle file, uint64 offset, FSHandle *offset_handle);

    //同步文件系统的附加维护信息（如果有，没有的话请赋值一个空的函数）
    int (*sync_information)(struct tagFSTool *fs);
}FSTool;

#define CREATEFILE_MODE_NEW     0
#define CREATEFILE_MODE_OPEN    1

inline int check_is_LE() {
    union {int x; char y; }u;
    u.x = 1; 
    return u.y == 1;
}

#ifdef __cplusplus
}
#endif

#endif