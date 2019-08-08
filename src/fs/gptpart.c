#include "gptpart.h"

unsigned int crc32(const void *pdata, unsigned int size)
{
    const unsigned char *data = (const unsigned char *)pdata;
    unsigned int crc = 0xffffffffu;
    for (unsigned int i = 0; i < size; i++)
    {
        crc = crc ^ data[i];
        for (int j = 8; j > 0; j--)
        {
             crc = (crc >> 1) ^ (0xEDB88320U & ((crc & 1) ? 0xFFFFFFFF : 0));
        }
    }
    return ~crc;
}

int gpt_is_parted(GPTItem *item) {
    return item->start_lba <= item->end_lba && memcmp(item->type_guid, GUID_EMPTY, sizeof(GPTGUID));
}

static uint32 _SEED_ = 32;    //随机数种子
void RandomGUID(GPTGUID *guid) {
    uint32 magic = ((uint64)guid);     //选取一个magic，指针先转换成uint64再赋给magic避免某些编译器报错
    uint32 offset = 0x12345; 
    for(int i = 0; i < 16; i++) {
        (*guid)[i] = (_SEED_ = (_SEED_ * magic + offset) % 255);
    }; 
}

int gpt_part_tool_initialize(GPTPartTool *pt, FSIO io) {
    pt->pt.io = io;

    pt->pt.create_part = gpt_create_part;
    pt->pt.delete_part = gpt_delete_part;
    pt->pt.enum_part = gpt_enum_part;
    pt->pt.get_part_count = gpt_get_part_count;
    pt->pt.query_part_info = gpt_query_part_info;
    pt->pt.sync_with_disk = gpt_sync_with_disk;
    pt->pt.make_part_table = gpt_make_part_table;
    memset(&pt->header, 0,  sizeof(GPTHeader));
    memset(pt->items, 0, sizeof(pt->items));
    pt->pt.io.read_lba(1, 1, &pt->header);
    if(memcmp(pt->header.signature, "EFI PART", 8)) {
        pt->is_valid_gpt_disk = 0;
    } else pt->is_valid_gpt_disk = 1;
    
    if(!pt->header.gpt_item_first_lba)      //for some buggy gpt header...
        pt->header.gpt_item_first_lba = 2;

    pt->num_items = pt->header.gpt_part_count > 128 ? 128 : pt->header.gpt_part_count;
    size_t read_size = (pt->num_items * sizeof(GPTItem));
    size_t read_count = read_size % 512 ? (read_size / 512 + 1) : read_size / 512;

    pt->pt.io.read_lba(pt->header.gpt_item_first_lba, pt->header.gpt_item_first_lba + read_count - 1, pt->items);
    
    if(pt->header.backup_lba) { //有备份表
        pt->pt.io.read_lba(pt->header.backup_lba, pt->header.backup_lba, &pt->header_backup);
    } else {
        memset(&pt->header_backup, 0, sizeof(GPTHeader));
    }
    
    return STATUS_SUCCESS;
}

#define CHECK_GPT_VALID if(!gpt->is_valid_gpt_disk) {return ERROR_TFS_INVALID_DISK;}

//内部使用，重做分区表crc32校验
void gpt_remake_crc32(GPTPartTool *gpt) {
    gpt->header.gpt_part_crc32 = crc32(gpt->items, sizeof(gpt->items));
    gpt->header.header_crc32 = 0;
    unsigned int crc = crc32(&gpt->header, 92);
    gpt->header.header_crc32 = crc;

    if(gpt->header.backup_lba) { //备份表
        memcpy(&gpt->header_backup, &gpt->header, sizeof(GPTHeader));
        gpt->header_backup.header_lba = gpt->header.backup_lba;
        gpt->header_backup.backup_lba = gpt->header.header_lba;
        gpt->header_backup.header_crc32 = 0;
        crc = crc32(&gpt->header_backup, 92);
        gpt->header_backup.header_crc32 = crc;
    }
}

#ifdef _WIN32
//Taurix patch: 在windows上使用mingw编译的时候使用原本的gpt_make_part_table会报错undefined reference to __chkstk_ms
//这里替换成如下函数通过编译
int gpt_make_part_table(struct tagPartTool *pt, uint64 disk_size_in_byte) {
    return ERROR_TFS_NO_IMPLEMENT;
}

#else 
int gpt_make_part_table(struct tagPartTool *pt, uint64 disk_size_in_byte) {
    GPTPartTool *gpt = (GPTPartTool *)pt;
    //   
    uint64 size_in_byte = disk_size_in_byte;
    uint64 last_lba = size_in_byte / 512 - 2;

    uint64 cur_lba = 0;
    {       //mbr
            char mbr[512];
            memset(mbr, 0, sizeof(mbr));
             
            mbr[0x1c0] = 0x01;  //fixed
            mbr[0x1c1] = 0x00;
            mbr[0x1c2] = 0xee;  //GPT signature
            mbr[0x1c3] = 0xfe;  //-|
            mbr[0x1c4] = 0xff;  // fixed
            mbr[0x1c5] = 0xff;  //
            mbr[0x1c6] = 0x01;  //-|
            mbr[0x1c7] = mbr[0x1c8] = mbr[0x1c9] = 0;
            unsigned long long selectors = (size_in_byte / 512 - 1) > 0xffffffffu ? 0xffffffffu : (size_in_byte / 512 - 1);
            *((unsigned int*)&mbr[0x1ca]) = (unsigned int)selectors;

            mbr[0x1fe] = 0x55;   
            mbr[0x1ff] = 0xaa;
            gpt->pt.io.write_lba(cur_lba, cur_lba, mbr);
            cur_lba++;
    }

    {       //gpt tables
            GPTHeader gh;
            GPTItem items[128];
            memset(&gh, 0, sizeof(gh));
            memset(items, 0, sizeof(items));
            memcpy(gh.signature, "EFI PART", 8);
            gh.version = 0x00010000;
            gh.sizeof_header = 92;
            gh.header_crc32 = 0;
            gh.reserved = 0;
            gh.header_lba = 1;
            gh.backup_lba = last_lba + 1; 
            //gh.backup_lba = (size_in_byte / 512 - 1);
            gh.first_partable_lba = 2048;
            gh.last_partable_lba = last_lba;
            RandomGUID(&gh.disk_guid);
            gh.gpt_item_first_lba = 2;
            gh.gpt_part_count = 128;
            gh.gpt_item_size = 128;
            
            //see also -->gpt_remake_crc32 
            gh.gpt_part_crc32 = crc32(items, sizeof(items));
            unsigned int header_crc32 = crc32(&gh, 92);
            gh.header_crc32 = header_crc32;
            gpt->pt.io.write_lba(cur_lba, cur_lba, &gh);
            cur_lba++;

            char zeros[512] = {0};
            for(int i = 2; i < 34; i++) {         //空表项
                gpt->pt.io.write_lba(cur_lba, cur_lba, zeros);
                cur_lba++;
            }

            for(int i = 34; i <= last_lba; i++) { //数据区填0
                gpt->pt.io.write_lba(cur_lba, cur_lba, zeros);
                cur_lba++;
            }

            //写入备份表
            gh.header_lba = gh.backup_lba;
            gh.backup_lba = 1;
            gh.header_crc32 = 0;
            header_crc32 = crc32(&gh, 92);
            gh.header_crc32 = header_crc32;
            gpt->pt.io.write_lba(cur_lba, cur_lba, &gh);
            cur_lba++;
    }
    gpt->is_valid_gpt_disk = 1; //设为合法的gpt磁盘
    return STATUS_SUCCESS;
}
#endif

unsigned int gpt_get_part_count(PartTool *pt) {
    GPTPartTool *gpt = (GPTPartTool *)pt;
    CHECK_GPT_VALID
    return (gpt)->num_items;
}

int gpt_create_part(PartTool *pt, PartInfo *info,FSHandle *target) {
    GPTPartTool *gpt = (GPTPartTool *)pt;
    CHECK_GPT_VALID
    if(info->start_lba > info->end_lba)
        return ERROR_TFS_INVALID_ARGUMENT;

    if(info->start_lba < gpt->header.first_partable_lba || info->end_lba > gpt->header.last_partable_lba) 
        return ERROR_TFS_NO_SPACE;
    
    GPTItem *item = NULL;
    for(int i = 0; i < 128; i++) {
        if(gpt_is_parted(&gpt->items[i])) {  //与已有的分区冲突
            GPTItem *p = &gpt->items[i];
            if(!(info->start_lba > p->end_lba || info->end_lba < p->start_lba)) {
                return ERROR_TFS_CONFLICT;            
            }
        } else {
            if(!item) {
                item = &gpt->items[i];
            } 
        }
    }
    if(item) {
        /* 
        if(info->type >= PART_UNKNOWN) 
            return ERROR_TFS_INVALID_ARGUMENT;
        */

        item->attributes = info->attribute;
        memcpy(item->type_guid, info->type_guid, sizeof(GPTGUID));
        memcpy(item->guid, info->guid, sizeof(GPTGUID));
        //int is_le = check_is_LE();    //GPT分区表label是规定的小端序的
        for(int i = 0; i < 36; i++) {
            item->label[i*2] = info->label[i];
        }
        item->label[71] = 0;

        item->start_lba = info->start_lba;
        item->end_lba = info->end_lba;
        
        gpt_remake_crc32(gpt);
        if(target) {
            *target = (FSHandle)item;
        }
        return STATUS_SUCCESS;
    }else {
        return ERROR_TFS_NO_SPACE;
    }

    return STATUS_SUCCESS;
}
int gpt_delete_part(PartTool *pt, FSHandle target) {
    GPTPartTool *gpt = (GPTPartTool *)pt;
    CHECK_GPT_VALID
    GPTItem *item = (GPTItem *)target;
    
    //不是从管理工具中得到的句柄
    if(item < gpt->items || item >= (gpt->items + gpt->num_items))
        return ERROR_TFS_INVALID_HANDLE;

    memset(item, 0, sizeof(GPTItem));
    gpt_remake_crc32(gpt);

    return STATUS_SUCCESS;
}

FSHandle gpt_enum_part(PartTool *pt, FSHandle last) {
    GPTPartTool *gpt = (GPTPartTool *)pt;
//    CHECK_GPT_VALID

    if(!gpt->is_valid_gpt_disk) 
        return 0;

    GPTItem *last_item = (GPTItem *)last;
    
    for(GPTItem *p = last_item ? (last_item + 1) : gpt->items; p < (gpt->items + gpt->num_items); p++) {
        if(gpt_is_parted(p)) {
            return (FSHandle)p;
        }
    }
    
    return 0;
}

int gpt_query_part_info(PartTool *pt, FSHandle target, PartInfo *info) {
    GPTPartTool *gpt = (GPTPartTool *)pt;
    CHECK_GPT_VALID

    if(target) {
        GPTItem *item = (GPTItem *)target;
        info->start_lba = item->start_lba;
        info->end_lba = item->end_lba;
        memcpy(info->guid, item->guid, sizeof(GPTGUID));
        memcpy(info->type_guid, item->type_guid, sizeof(GPTGUID));
        for(int i = 0; i < 36; i++) {
            info->label[i] = item->label[i*2];
        }
        info->label[35] = 0;
        for(int i = 0; i < PART_UNKNOWN; i++) {
            if(!memcmp(PART_TYPE_GUID(i), item->type_guid, sizeof(GPTGUID))) {
                info->type = PART_EMPTY + i;
                return STATUS_SUCCESS;
            }
        }
        info->type = PART_UNKNOWN;
        return STATUS_SUCCESS;
    } else {
        return STATUS_FAILED;
    }

    return STATUS_SUCCESS;
}



int gpt_sync_with_disk(struct tagPartTool *pt) {
    GPTPartTool *gpt = ((GPTPartTool *)pt);
    CHECK_GPT_VALID

    gpt_remake_crc32(gpt);

    //主表表头
    gpt->pt.io.write_lba(1, 1, &gpt->header);
    //主表表项
    gpt->num_items = gpt->header.gpt_part_count > 128 ? 128 : gpt->header.gpt_part_count;
    size_t read_size = (gpt->num_items * sizeof(GPTItem));
    size_t read_count = read_size % 512 ? (read_size / 512 + 1) : read_size / 512;
    gpt->pt.io.write_lba(gpt->header.gpt_item_first_lba, gpt->header.gpt_item_first_lba + read_count - 1, gpt->items); 

    //备份表
    if(gpt->header.backup_lba) {
        gpt->pt.io.write_lba(gpt->header.backup_lba, gpt->header.backup_lba, &gpt->header_backup);
    }
    return STATUS_SUCCESS;
}

#undef CHECK_GPT_VALID
