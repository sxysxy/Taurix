#include "fat32.h"

static inline uint64 cluster2lba(FAT32Tool *fat, uint32 cluster) {
    uint64 data_lba = fat->fat_lba + fat->num_fat * fat->sectors_per_fat;
    return data_lba + (cluster-2) * fat->sectors_per_cluster;  
            //(cluster-2)而不是cluster，因为前两个簇被保留了。FAT32只是保留前2个FAT表项，并不保留前两个簇。
                //gtmd被wiki的中文翻译坑了。。。
} 

//将fat表第fatid项的值设置为newid，如果newid小于0则不改变fatid项上的数值，为0是空闲簇
//如果oldid不为空指针，会设置通过这个参数返回这个表项上原本的数值
static int set_fat_next(FAT32Tool *fat, uint32 fatid, uint32 *oldid, int newid) {
    uint64 lba = fat->fat_lba + fatid / 128;
    if(lba < fat->fat_lba || lba >= fat->fat_lba + fat->sectors_per_fat) 
        return ERROR_TFS_INVALID_ARGUMENT;
    fat->fs.io.read_lba(lba, lba, fat->sector_buffer);
    uint32* fats = (uint32*)fat->sector_buffer;
    if(oldid)
        *oldid = fats[fatid % 128];
    if(newid >= 0) {
        fats[fatid % 128] = newid;
        for(int i = 0; i < fat->num_fat; i++) { //写回修改后的fat表项
            fat->fs.io.write_lba(lba + fat->sectors_per_fat * i, lba + fat->sectors_per_fat * i, fat->sector_buffer); 
        }
    }
    return STATUS_SUCCESS;
}

static uint32 __find_next_free_item(FAT32Tool *fat) {

    static uint32 fatid = 0;  //查找的起点
    int wrapped = 0; //回头从前面找过的标记

    while(1) {
        int flag = 0;
        uint64 lba = fat->fat_lba + fatid / 128;
        fat->fs.io.read_lba(lba, lba, fat->sector_buffer);
        uint32 *fats = (uint32*)fat->sector_buffer;
        int offset = 0;
        for(int i = fatid % 128; i < 128; i++) {
            if(!fats[i]) {
                flag = 1;
                offset = i;
                break;
            } else fatid++;  
                
        }
        if(flag) {  //已找到
            return fatid;  //返回fat表项的id，也即空闲簇的簇号  
        } else {
            lba++;
            if(lba == fat->fat_lba + fat->sectors_per_fat) {
                if(!wrapped) {
                    fatid = 0;
                    wrapped = 1;
                    continue;
                } else {
                    break; //确实没有空闲表项了
                }
            }
        }
    }
    return 0;
}

//找到一个空闲表项(空闲簇），分配掉，返回编号，如果返回0表示fat表被分配完
static uint32 alloc_fat_item(FAT32Tool *fat) {
    const uint32 cluster = fat->fsinfo.next_free_cluster;
    if(cluster) {
        const uint64 lba = fat->fat_lba + cluster / 128;  //因为一个扇区有128个FAT表项
        fat->fs.io.read_lba(lba, lba, fat->sector_buffer);
        uint32 *fats = (uint32*)fat->sector_buffer;
        fats[cluster % 128] = FAT32_FILE_END;
        fat->fs.io.write_lba(lba, lba, fat->sector_buffer);  //写回
        fat->fsinfo.next_free_cluster = __find_next_free_item(fat);
        fat->fsinfo.free_clusters--;
    }
    return cluster; 
}

int fat32_fstool_initialize(FAT32Tool *fat, FSIO io, uint64 start_lba, uint64 end_lba) {
    fat->fs.io = io;
    if(start_lba > end_lba) 
        return ERROR_TFS_INVALID_ARGUMENT;
    fat->fs.start_lba = start_lba;
    fat->fs.end_lba = end_lba;
    fat->fs.create_file = fat32_create_file;
    fat->fs.delete_file = fat32_delete_file;
    fat->fs.makefs = fat32_makefs;
    fat->fs.enum_dir = fat32_enum_dir;
    fat->fs.query_file_info = fat32_query_file_info;
    fat->fs.create_file = fat32_create_file;
    fat->fs.delete_file = fat32_delete_file;
    fat->fs.fread = fat32_fread;
    fat->fs.fwrite = fat32_fwrite;
    fat->fs.fseek = fat32_fseek;
    fat->fs.sync_information = fat32_sync_information;

    FAT32Header header;
    fat->fs.io.read_lba(start_lba, start_lba, &header);
    fat->is_valid_fat32 = memcmp("FAT32", header.fat32, 5) == 0;
    
    fat->fat_lba = fat->fs.start_lba + header.num_reserved; 
    fat->num_fat = header.num_fat;
    fat->sectors_per_fat = header.sectors_per_fat32;
    fat->sectors_per_cluster = header.sectors_per_clust;
    fat->root_dir_cluster = header.root_dir_start;
    //fat->root_dir_cluster = 0;
    //fsinfo
    if(header.fsinfo) {
        fat->fs.io.read_lba(start_lba + header.fsinfo, start_lba + header.fsinfo, &fat->fsinfo );
    } 
    if(fat->sectors_per_cluster > 8) {   //大于8不支持鸭
        return ERROR_TFS_UNSUPPORTED_FEATURE;
    }
    if(((uint64)fat->sectors_per_fat) * 512 / 4 > 0xfffffef) {
        return ERROR_TFS_NO_SPACE;  //fat表表项太多
    }
    return STATUS_SUCCESS;
}

int fat32_makefs(struct tagFSTool *fs) {
    FAT32Tool *fat = ((FAT32Tool*)fs);
    if(fs->end_lba - fs->start_lba + 1 < 4000)
        return ERROR_TFS_NO_SPACE;
    if(fs->end_lba - fs->start_lba + 1 > 0xffffffffu) //太大
        return ERROR_TFS_NO_SPACE;
    {
    FAT32Header header;
    memset(&header, 0, sizeof(header));
    memcpy(header.jmp, "\xeb\x58\x90", 3);
    memcpy(header.OEM, "tfstool", 8);
    header.bytes_per_sector = 512;
    //header.sectors_per_clust = 8; //4kb
    header.sectors_per_clust = 1;
    header.num_reserved = 32;
    header.num_fat = 2;                 //两个fat表
    header.material = 0xf8;
    header.num_sectors = fs->end_lba - fs->start_lba + 1;
    //s <- end_lba - start_lba + 1 - num_reserved
    //t <- sectors_per_clust
    //num_fat = 2
    //x <- sectors_per_fat32
    //sectors_per_clust 
    //s - 2 * x = x * (512/4) * t
    //        x = s / (128*t + 2)
    header.sectors_per_fat32 = (uint32)(fs->end_lba - fs->start_lba + 1 - header.num_reserved) / (128*header.sectors_per_clust+2) - 1;
    if(header.sectors_per_fat32 < header.num_fat + 1)
        return ERROR_TFS_NO_SPACE;
    header.root_dir_start = 2;  //
    header.fsinfo = 1;
    header.backup = 2;
    header.device_code = 0x80;
    memcpy(header.fat32, "FAT32", 5);
    header.aa55 = 0xaa55;
    fs->io.write_lba(fs->start_lba, fs->start_lba, &header);
    fs->io.write_lba(fs->start_lba+2, fs->start_lba+2, &header);  //备份引导扇区

    //fsinfo:
    fat->fsinfo.aa55 = 0xaa55;
    fat->fsinfo.x41615252 = 0x41615252;
    fat->fsinfo.x61417272 = 0x61417272;
    fat->fsinfo.free_clusters = (uint32)((fs->end_lba - fs->start_lba + 1) - header.sectors_per_fat32 * header.num_fat) / header.sectors_per_clust;
    //fat->fsinfo.next_free_cluster = header.root_dir_start + 1;
    fat->fsinfo.next_free_cluster = 5;
    fs->io.write_lba(fs->start_lba+1, fs->start_lba+1, &fat->fsinfo); 
    }

    //在这之前fat结构体内的成员大都是不可用的
    int e = fat32_fstool_initialize(fat, fs->io, fs->start_lba, fs->end_lba);  //偷个懒，直接调用它...
    if(e) return e;
    {
    unsigned char init_fat[512] = {0};
    //0号项
    init_fat[0] = 0xf8;
    init_fat[1] = 0xff;
    init_fat[2] = 0xff;
    init_fat[3] = 0x0f;
    //1号项
    init_fat[4] = 0xf8;
    init_fat[5] = 0xff;
    init_fat[6] = 0xff;
    init_fat[7] = 0x0f;
    //根目录项
    init_fat[8] = 0xff;
    init_fat[9] = 0xff;
    init_fat[10] = 0xff;
    init_fat[11] = 0x0f;
    fs->io.write_lba(fat->fat_lba, fat->fat_lba, init_fat);  //FAT1
    fs->io.write_lba(fat->fat_lba + fat->sectors_per_fat, fat->fat_lba + fat->sectors_per_fat, init_fat); //FAT2
    }
    return e;
}                                                //创建无文件的文件系统

#define CHECK_FS if(!fat->is_valid_fat32) {return ERROR_TFS_INVALID_FS; }

#define HANDLE_CLUSTER(h) ((h) & 0x00000000ffffffffull)
#define HANDLE_OFFSET(h)  (((h) & 0xffffffff00000000ull) >> 32)
#define MAKE_CLUSTER_OFFSET_HANDLE(cl, of) (((FileHandle)of << 32) | cl)

//FileItem可被占用
#define ITEM_AVAILABLE(x) (x.attribute == 0 || x.dosname[0] == 0xe5)  

//将FileInfo指示的信息转变为Fat文件系统的文件信息，返回错误信息
static inline int FileInfoToFat(FileInfo *info, FAT32FileItem *item, uint32 first_cluster) {
    int name_len = strlen(info->name_char);
    int i;
    for(i = name_len - 1; ~i; i--) {
        if(info->name_char[i] == '.') {  //找最后一个 .
            if(i <= 7) {
                if(name_len - i - 1 > 3) {
                    return ERROR_TFS_INVALID_FILENAME;  //扩展名太长
                } else {
                    break;
                }
            } else {
                return ERROR_TFS_INVALID_FILENAME;  //文件名太长
            }
        }   
    }
    
    if(i < 0) { //没有扩展名 
        if(name_len > 8) return ERROR_TFS_INVALID_FILENAME;  //太长
        strcpy((char*)item->dosname, info->name_char);
        for(i = name_len; i < 8; i++) { //空格补齐
            item->dosname[i] = ' ';  
        }
        memset(item->dosname_ext, ' ', sizeof(item->dosname_ext));
    } else {
        if(i > 8) 
            return ERROR_TFS_INVALID_FILENAME; //太长
        strncpy((char*)item->dosname, info->name_char, i);
        for(int j = i; j < 8; j++)
            item->dosname[j] = ' ';
        int ext_len = name_len - i - 1;
        strncpy((char*)item->dosname_ext, info->name_char+i+1, ext_len);
        for(int j = ext_len; j < 3; j++)
            item->dosname_ext[j] = ' ';  //补空格
    }

    item->attribute = 0;
    if(info->attributes & FILE_ATTRIBUTE_READONLY) 
        item->attribute |= 0x01;
    if(info->attributes & FILE_ATTRIBUTE_HIDDEN) 
        item->attribute |= 0x02;
    if(info->attributes & FILE_ATTRIBUTE_SYSTEM) 
        item->attribute |= 0x04;
    if(info->is_directory) {
        item->attribute |= 0x10;
    } else {
        item->attribute |= 0x20; 
    }
    if(info->attributes & FILE_ATTRIBUTE_DEVICE) 
        item->attribute |= 0x40;

    item->create_time_ymd = (((info->created_year - 1980) & 0x7f) << 9) | (info->created_month << 5) | (info->created_day);
    item->create_time_hmsdiv2 = (info->created_hour << 11) | (info->created_minute << 5) | (info->created_second / 2);
    item->create_time_in2s = (info->created_second & 1) * 100;

    item->access_time_ymd = (((info->access_year - 1980) & 0x7f) << 9) | (info->access_month << 5) | (info->access_day);
    item->modify_time_ymd = (((info->modify_year - 1980) & 0x7f) << 9) | (info->modify_month << 5) | (info->modify_day);
    item->modify_time_hmsdiv2 = (info->modify_hour << 11) | (info->modify_minute << 5) | (info->modify_second / 2);
    if(info->file_size_in_bytes > 0xffffffffu) 
        return ERROR_TFS_NO_SPACE;  //文件太大。。。
    item->file_size_in_byte = info->file_size_in_bytes;
    item->first_cluster_low = first_cluster & 0x0000ffff;
    item->first_cluster_hight = (first_cluster & 0xffff0000u) >> 16;
    return STATUS_SUCCESS;
}

//Fat32目录项 -> 标准FileInfo，无需返回值
void FileItemToFileInfo(FAT32Tool *fat, FAT32FileItem *item, FileInfo *info) {
    //size
    info->file_size_in_bytes = item->file_size_in_byte;
    int size_per_cluster = (512 * fat->sectors_per_cluster);
    info->file_occupies_in_bytes = item->file_size_in_byte % size_per_cluster ? (item->file_size_in_byte / size_per_cluster + 1) * size_per_cluster : item->file_size_in_byte; 

    //attribute
    if(item->attribute & 0x10) {
        info->is_directory = 1; 
    } else info->is_directory = 0;
    if(item->attribute & 0x40) 
        info->attributes |= FILE_ATTRIBUTE_DEVICE;
    if(item->attribute & 0x01)
        info->attributes |= FILE_ATTRIBUTE_READONLY;
    if(item->attribute & 0x02) 
        info->attributes |= FILE_ATTRIBUTE_HIDDEN; 
    if(item->attribute & 0x04)
        info->attributes |= FILE_ATTRIBUTE_SYSTEM;

    //name
    int name_len = 8;
    for(name_len--; name_len >= 0 && item->dosname[name_len] == ' '; name_len--);
    name_len++;

    memcpy(info->name_char, (const void*)item->dosname, name_len);
    info->name_char[name_len] = 0;

    int ext_len = 3;
    for(ext_len--; ext_len>=0 && item->dosname_ext[ext_len] == ' '; ext_len--);
    ext_len++;
    if(ext_len) {
        strcat(info->name_char, ".");
        memcpy(info->name_char + name_len + 1, item->dosname_ext, ext_len);
        info->name_char[name_len + 1 + ext_len] = 0; 
    }
    //TODO: 日期信息，太智障了不想做了。。。恶心死了
    info->access_ymdhms = 0;
    info->modify_ymdhms = 0;
    info->created_ymdhms = 0;

    //return;
}

static inline uint64 __u64min(uint64 x, uint64 y) {
    return x < y ? x : y;
}


int fat32_create_file(struct tagFSTool *fs,  FSHandle dir, FileInfo *info, FSHandle *handle) {
    FAT32Tool *fat = ((FAT32Tool*)fs);
    CHECK_FS

    int num = fat->sectors_per_cluster * 0x200 / 0x20; //单个簇内目录项个数

    uint64 current_cluster;
    if(dir != FSTOOL_INVALID_HANDLE) {
        current_cluster = HANDLE_CLUSTER(dir);
        fs->io.read_lba(cluster2lba(fat, current_cluster), cluster2lba(fat, current_cluster) + fat->sectors_per_cluster - 1, 
            fat->cluster_buffer);
        uint64 idx = HANDLE_OFFSET(dir);
        FAT32FileItem *items = (FAT32FileItem*) fat->cluster_buffer;
        if(idx >= num || idx < 0) return ERROR_TFS_INVALID_HANDLE;
        if(!(items[idx].attribute & 0x10)) {  //不是目录项
            return ERROR_TFS_INVALID_HANDLE;
        }
        current_cluster = items[idx].first_cluster_low | (items[idx].first_cluster_hight << 16);
    } else {
        current_cluster = fat->root_dir_cluster;
    }

    {
        fs->io.read_lba(cluster2lba(fat, current_cluster), cluster2lba(fat, current_cluster) + fat->sectors_per_cluster - 1, fat->cluster_buffer);
        FAT32FileItem *items = (FAT32FileItem *)fat->cluster_buffer;
        while(1) {
            int flag = 0;
            
            int i; 
            for(i = 0; i < num; i++) {
                if(ITEM_AVAILABLE(items[i])) {
                    flag = 1;
                    break;
                } 
            }
            if(flag) {  //找到空目录项
                uint32 free_cluster = alloc_fat_item(fat);  //为新文件分配一个簇
                if(!free_cluster)
                    return ERROR_TFS_NO_SPACE; 
                int err = FileInfoToFat(info, items+i, free_cluster);
                if(err) return err; 
                fs->io.write_lba(cluster2lba(fat, current_cluster), cluster2lba(fat, current_cluster) + fat->sectors_per_cluster - 1, fat->cluster_buffer);
                if(handle) {
                    *handle = MAKE_CLUSTER_OFFSET_HANDLE(current_cluster, i);  //文件记录占用当前簇的第i个表项
                }
                if(info->is_directory) {  //创建的是子目录，在所创建的目录中创建'.'和'..'的文件记录
                    memset(fat->sector_buffer, 0, sizeof(fat->sector_buffer));
                    items = (FAT32FileItem *)fat->sector_buffer;
                    memset(items, 0x20, sizeof(FAT32FileItem));
                    memset(items+1, 0x20, sizeof(FAT32FileItem));
                    items[0].dosname[0] = 0x2e;
                    items[1].dosname[0] = 0x2e;
                    items[1].dosname[1] = 0x2e;
                    items[0].attribute = 0x10;
                    items[1].attribute = 0x10;
                    fs->io.write_lba(cluster2lba(fat, free_cluster), cluster2lba(fat, free_cluster), fat->sector_buffer);
                }
                return STATUS_SUCCESS;
            } else {
                uint32 next_cluster;
                set_fat_next(fat, current_cluster, &next_cluster, -1);
                if(FAT32_IS_END_CLUSTAR(next_cluster)) {
                    uint32 free_cluster = alloc_fat_item(fat); //新开一个簇储存目录内的信息，那么下一次一定可以找得到。。。
                    if(!free_cluster)  return ERROR_TFS_NO_SPACE;
                    set_fat_next(fat, current_cluster, NULL, free_cluster);
                    current_cluster = free_cluster;
                } else {
                    current_cluster = next_cluster;
                }
            }
        }
    } 

    return STATUS_SUCCESS;
}                        //创建文件

int fat32_delete_file(struct tagFSTool *fs, FSHandle target) {
    FAT32Tool *fat = ((FAT32Tool*)fs);
    CHECK_FS
    
    int num = fat->sectors_per_cluster * 0x200 / 0x20; //单个簇内目录项个数

    if(target == FSTOOL_INVALID_HANDLE) {
        return ERROR_TFS_INVALID_HANDLE;
    }

    uint64 current_cluster = HANDLE_CLUSTER(target);

    //进入目录所在簇，得到对应目录项
    {
        fs->io.read_lba(cluster2lba(fat, current_cluster), cluster2lba(fat, current_cluster) + fat->sectors_per_cluster - 1, 
            fat->cluster_buffer);
        uint64 idx = HANDLE_OFFSET(target);
        FAT32FileItem *items = (FAT32FileItem*) fat->cluster_buffer;
        if(idx >= num || idx < 0) return ERROR_TFS_INVALID_HANDLE;
        items[idx].dosname[0] = 0xe5;  //标记删除
        fs->io.write_lba(cluster2lba(fat, current_cluster), cluster2lba(fat, current_cluster) + fat->sectors_per_cluster - 1, 
            fat->cluster_buffer);
    }

    return STATUS_SUCCESS;
}                    //删除文件

int fat32_query_file_info(struct tagFSTool *fs, FSHandle file, FileInfo *info) {
    FAT32Tool *fat = ((FAT32Tool*)fs);
    CHECK_FS
    
    int num = fat->sectors_per_cluster * 0x200 / 0x20; //单个簇内目录项个数

    if(file == FSTOOL_INVALID_HANDLE) {
        return ERROR_TFS_INVALID_HANDLE;
    }

    uint64 current_cluster = HANDLE_CLUSTER(file);

    {
        fs->io.read_lba(cluster2lba(fat, current_cluster), cluster2lba(fat, current_cluster) + fat->sectors_per_cluster - 1, 
            fat->cluster_buffer);
        uint64 idx = HANDLE_OFFSET(file);
        FAT32FileItem *items = (FAT32FileItem*) fat->cluster_buffer;
        if(idx >= num || idx < 0) return ERROR_TFS_INVALID_HANDLE;
        FileItemToFileInfo(fat, &items[idx], info);
    }

    return STATUS_SUCCESS;
}

FSHandle fat32_enum_dir(struct tagFSTool *fs, FSHandle dir, FSHandle last, FileInfo *info){
    FAT32Tool *fat = ((FAT32Tool*)fs);
    if(!fat->is_valid_fat32) 
        return 0;
    
    int num = fat->sectors_per_cluster * 0x200 / 0x20; //单个簇内目录项个数
    uint32 cluster;
    uint32 offset;
    
    if(FSTOOL_INVALID_HANDLE == last) {  //第一次
        if(dir != FSTOOL_INVALID_HANDLE) {
            uint64 current_cluster = dir ? HANDLE_CLUSTER(dir) : fat->root_dir_cluster;
            fs->io.read_lba(cluster2lba(fat, current_cluster), cluster2lba(fat, current_cluster) + fat->sectors_per_cluster - 1, 
                    fat->cluster_buffer);
            uint64 idx = HANDLE_OFFSET(dir);
            FAT32FileItem *items = (FAT32FileItem*) fat->cluster_buffer;
            if(idx >= num || idx < 0) return ERROR_TFS_INVALID_HANDLE;
            if(!(items[idx].attribute & 0x10))   //不是目录项
                return FSTOOL_INVALID_HANDLE;
        
            current_cluster = items[idx].first_cluster_low | (items[idx].first_cluster_hight << 16);
            cluster = current_cluster;
            offset = 0;
        } else {
            cluster = fat->root_dir_cluster;
            offset = 0;
        }
    } else {  //不是第一次，接着上次的Handle(last)往后
        cluster = HANDLE_CLUSTER(last);
        offset = HANDLE_OFFSET(last) + 1;

        if(offset == num) { //跳到指向的下一块cluster
            uint32 next_cluster;
            set_fat_next(fat, cluster, &next_cluster, -1); 
            cluster = next_cluster;
            if(FAT32_IS_END_CLUSTAR(cluster)) { //到目录项最后一个簇了
                return FSTOOL_INVALID_HANDLE;
            }
            offset = 0;
        }
    }
    if(offset < 0 || offset >= num)
        return FSTOOL_INVALID_HANDLE;
    
    //判断是否到末尾
    fs->io.read_lba(cluster2lba(fat, cluster), cluster2lba(fat, cluster) + fat->sectors_per_cluster - 1,
        fat->cluster_buffer);
    FAT32FileItem *items = (FAT32FileItem *)fat->cluster_buffer;
    if(items[offset].dosname[0] != 0xe5 && items[offset].attribute) {  //有效 
        //返回指向它的句柄
        if(info) {
            FileItemToFileInfo(fat, items + offset, info);
        }
        return MAKE_CLUSTER_OFFSET_HANDLE(cluster, offset);  //返回指向下一个文件的句柄
    } else {  //无效
        return FSTOOL_INVALID_HANDLE;
    }
}       //列举目录dir内文件(包括子目录)，第一次last为NULL

uint64 fat32_fread(struct tagFSTool *fs, FSHandle file, const void *buffer, uint64 buffer_size, FSHandle *offset_handle) {
    FAT32Tool *fat = ((FAT32Tool*)fs);
    if(!fat->is_valid_fat32)
        return 0;

    if(file == FSTOOL_INVALID_HANDLE)
        return 0;
      //复制粘贴过来的呀
    uint64 file_size;
    uint32 cluster, foffset;
    {
        const int num = fat->sectors_per_cluster * 0x200 / 0x20; //单个簇内目录项个数
        cluster = HANDLE_CLUSTER(file);
        fs->io.read_lba(cluster2lba(fat, cluster), cluster2lba(fat, cluster) + fat->sectors_per_cluster - 1, 
                    fat->cluster_buffer);
        uint64 idx = HANDLE_OFFSET(file);
        FAT32FileItem *items = (FAT32FileItem*) fat->cluster_buffer;
        if(idx >= num || idx < 0) return ERROR_TFS_INVALID_HANDLE;
        
        file_size = items[idx].file_size_in_byte;
        cluster = items[idx].first_cluster_low | (items[idx].first_cluster_hight << 16);
        foffset = 0;
    }

    const uint32 cluster_size = fat->sectors_per_cluster * 0x200;
    if(*offset_handle) { //从偏移位置继续
        cluster = HANDLE_CLUSTER(*offset_handle);
        foffset = HANDLE_OFFSET(*offset_handle);
    }

    uint32 offset = foffset % cluster_size;   //这个offset是簇内偏移量
    uint64 buffer_offset = 0;  //buffer_offset同时也等于已读取的字节数
    int is_new = 0;     //当前簇是整块的簇的标记
    while(buffer_offset < buffer_size && foffset < file_size) {
      
        uint64 read_size = 0;   //本次读取的大小

        if(!is_new) {  
            read_size = __u64min(buffer_size - buffer_offset, cluster_size - offset);   //min(剩余数据量, 簇剩余空间)
            read_size = __u64min(read_size, file_size - foffset);   //再与文件剩余字节数比较
        } else { //整块的簇，直接从头开始读
            read_size = __u64min(cluster_size, buffer_size - buffer_offset);
            read_size = __u64min(read_size, file_size - foffset);
        }

        fs->io.read_lba(cluster2lba(fat, cluster), cluster2lba(fat, cluster) + fat->sectors_per_cluster - 1,
                fat->cluster_buffer);
        memcpy((char*)buffer + buffer_offset, &fat->cluster_buffer[offset], read_size);
        offset = (uint32)(offset + read_size) % cluster_size;
        buffer_offset += read_size;

        if(buffer_offset >= buffer_size || foffset >= file_size) 
            break;
        if(!offset) {  //找下一块簇 
            uint32 next_cluster;
            set_fat_next(fat, cluster, &next_cluster, -1);
            if(FAT32_IS_END_CLUSTAR(next_cluster)) {  //往后没有了
                break;
            } else {
                cluster = next_cluster;
            }
        }
    }
    *offset_handle = MAKE_CLUSTER_OFFSET_HANDLE(cluster, foffset + buffer_offset);
    return buffer_offset;
}

//offset_handle组成：
//簇号和相对于文件第0个字节的偏移量。
uint64 fat32_fwrite(struct tagFSTool *fs, FSHandle file, const void *data, uint64 data_size, FSHandle *offset_handle) {
    FAT32Tool *fat = ((FAT32Tool*)fs);
    if(!fat->is_valid_fat32)
        return 0;
    if(file == FSTOOL_INVALID_HANDLE)
        return 0;
    
    //复制粘贴过来的呀
    uint64 file_size;
    uint32 cluster, foffset;
    {
        const int num = fat->sectors_per_cluster * 0x200 / 0x20; //单个簇内目录项个数
        cluster = HANDLE_CLUSTER(file);
        fs->io.read_lba(cluster2lba(fat, cluster), cluster2lba(fat, cluster) + fat->sectors_per_cluster - 1, 
                    fat->cluster_buffer);
        uint64 idx = HANDLE_OFFSET(file);
        FAT32FileItem *items = (FAT32FileItem*) fat->cluster_buffer;
        if(idx >= num || idx < 0) return ERROR_TFS_INVALID_HANDLE;
        
        file_size = items[idx].file_size_in_byte;
        cluster = items[idx].first_cluster_low | (items[idx].first_cluster_hight << 16);
        foffset = 0;
    }

    const uint32 cluster_size = fat->sectors_per_cluster * 0x200;
    if(*offset_handle) { //从偏移位置继续
        cluster = HANDLE_CLUSTER(*offset_handle);
        foffset = HANDLE_OFFSET(*offset_handle);
    }

    uint32 offset = foffset % cluster_size;   //这个offset是簇内偏移量
    uint64 buffer_offset = 0;  //buffer_offset同时也等于已写入的字节数
    int is_new = 0;     //当前簇是新开的簇的标记
    while(buffer_offset < data_size) {
        
        //int is_last = data_size - buffer_offset <= cluster_size;    //当前簇是要写入的最后一块的标记
        uint64 write_size = 0;   //本次写入的大小

        if(!is_new) {  //先尽量把这个簇写满
            fs->io.read_lba(cluster2lba(fat, cluster), cluster2lba(fat ,cluster) + fat->sectors_per_cluster - 1,
                fat->cluster_buffer);
            write_size = __u64min(data_size - buffer_offset, cluster_size - offset);   //min(剩余数据量, 簇剩余空间)
            memcpy(&fat->cluster_buffer[offset], (char*)data + buffer_offset, write_size);
        } else { //新的簇，直接从头开始写
            write_size = __u64min(cluster_size, data_size - buffer_offset);
            memcpy(fat->cluster_buffer, (char*)data + buffer_offset, write_size);
        }

        offset = (uint32)(offset + write_size) % cluster_size;
        fs->io.write_lba(cluster2lba(fat, cluster), cluster2lba(fat, cluster) + fat->sectors_per_cluster - 1,
                fat->cluster_buffer);
        buffer_offset += write_size;

        if(buffer_offset >= data_size) 
            break;

        if(!offset && foffset >= file_size) {  //新开块
            uint32 new_cluster = alloc_fat_item(fat);
            if(!new_cluster)
                break;
            set_fat_next(fat, cluster, NULL, new_cluster);
            cluster = new_cluster;
            is_new = 1;
        } 
    }
    //重算文件大小
    uint32 add_bytes = data_size - (file_size - foffset);
    file_size += add_bytes;
    {
        uint32 tmp = HANDLE_CLUSTER(file), idx = HANDLE_OFFSET(file);
        fs->io.read_lba(cluster2lba(fat, tmp), cluster2lba(fat, tmp) + fat->sectors_per_cluster - 1, 
                    fat->cluster_buffer);
        FAT32FileItem *items = (FAT32FileItem*)fat->cluster_buffer;
        items[idx].file_size_in_byte = file_size;
        fs->io.write_lba(cluster2lba(fat, tmp), cluster2lba(fat, tmp) + fat->sectors_per_cluster - 1, 
                    fat->cluster_buffer);
    }

    *offset_handle = MAKE_CLUSTER_OFFSET_HANDLE(cluster, foffset + buffer_offset);
    return buffer_offset;
}
    
int fat32_fseek(struct tagFSTool *fs, FSHandle file, uint64 offset, FSHandle *offset_handle) {
    FAT32Tool *fat = ((FAT32Tool*)fs);
    CHECK_FS



    return ERROR_TFS_NO_IMPLEMENT;
}


//主要是同步fsinfo
int fat32_sync_information(struct tagFSTool *fs) {
    FAT32Tool *fat = ((FAT32Tool*)fs);
    fat->fs.io.read_lba(fs->start_lba, fs->start_lba, fat->sector_buffer);
    FAT32Header *header = (FAT32Header *)fat->sector_buffer;
    fat->fs.io.write_lba(fs->start_lba + header->fsinfo, fs->start_lba + header->fsinfo, &fat->fsinfo);
    return STATUS_SUCCESS;
}