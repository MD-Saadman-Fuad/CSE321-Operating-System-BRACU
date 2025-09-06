#define _FILE_OFFSET_BITS 64
#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <endian.h>
#include <stddef.h>

#define BS 4096u
#define INODE_SIZE 128u
#define ROOT_INO 1u
#define DIRECT_MAX 12
#define MAGIC_NUMBER 0x4D565346u
#define GROUP_ID 14

#pragma pack(push, 1)
typedef struct {
    uint32_t magic;
    uint32_t version;
    uint32_t block_size;
    uint64_t total_blocks;
    uint64_t inode_count;
    uint64_t inode_bitmap_start;
    uint64_t inode_bitmap_blocks;
    uint64_t data_bitmap_start;
    uint64_t data_bitmap_blocks;
    uint64_t inode_table_start;
    uint64_t inode_table_blocks;
    uint64_t data_region_start;
    uint64_t data_region_blocks;
    uint64_t root_inode;
    uint64_t mtime_epoch;
    uint32_t flags;
    uint32_t checksum;
} superblock_t;
#pragma pack(pop)
_Static_assert(sizeof(superblock_t) == 116, "superblock struct size mismatch");

#pragma pack(push,1)
typedef struct {
    uint16_t mode;
    uint16_t links;
    uint32_t uid;
    uint32_t gid;
    uint64_t size_bytes;
    uint64_t atime;
    uint64_t mtime;
    uint64_t ctime;
    uint32_t direct[12];
    uint32_t reserved_0;
    uint32_t reserved_1;
    uint32_t reserved_2;
    uint32_t proj_id;
    uint32_t uid16_gid16;
    uint64_t xattr_ptr;
    uint64_t inode_crc;
} inode_t;
#pragma pack(pop)
_Static_assert(sizeof(inode_t) == INODE_SIZE, "inode size mismatch");

#pragma pack(push,1)
typedef struct {
    uint32_t inode_no;
    uint8_t  type;
    char     name[58];
    uint8_t  checksum;
} dirent64_t;
#pragma pack(pop)
_Static_assert(sizeof(dirent64_t) == 64, "dirent size mismatch");

uint32_t CRC32_TAB[256];
void crc32_init(void){
    for (uint32_t i=0;i<256;i++){
        uint32_t c=i;
        for(int j=0;j<8;j++) c = (c&1)?(0xEDB88320u^(c>>1)):(c>>1);
        CRC32_TAB[i]=c;
    }
}
uint32_t crc32(const void* data, size_t n){
    const uint8_t* p=(const uint8_t*)data; uint32_t c=0xFFFFFFFFu;
    for(size_t i=0;i<n;i++) c = CRC32_TAB[(c^p[i])&0xFF] ^ (c>>8);
    return c ^ 0xFFFFFFFFu;
}

static uint32_t superblock_crc_finalize(superblock_t *sb) {
    superblock_t tmp = *sb;
    tmp.magic = htole32(tmp.magic);
    tmp.version = htole32(tmp.version);
    tmp.block_size = htole32(tmp.block_size);
    tmp.total_blocks = htole64(tmp.total_blocks);
    tmp.inode_count = htole64(tmp.inode_count);
    tmp.inode_bitmap_start = htole64(tmp.inode_bitmap_start);
    tmp.inode_bitmap_blocks = htole64(tmp.inode_bitmap_blocks);
    tmp.data_bitmap_start = htole64(tmp.data_bitmap_start);
    tmp.data_bitmap_blocks = htole64(tmp.data_bitmap_blocks);
    tmp.inode_table_start = htole64(tmp.inode_table_start);
    tmp.inode_table_blocks = htole64(tmp.inode_table_blocks);
    tmp.data_region_start = htole64(tmp.data_region_start);
    tmp.data_region_blocks = htole64(tmp.data_region_blocks);
    tmp.root_inode = htole64(tmp.root_inode);
    tmp.mtime_epoch = htole64(tmp.mtime_epoch);
    tmp.flags = htole32(tmp.flags);

    uint8_t block[BS];
    memset(block, 0, BS);
    memcpy(block, &tmp, sizeof(tmp));

    size_t checksum_offset = offsetof(superblock_t, checksum);
    if (checksum_offset + sizeof(tmp.checksum) <= BS) {
        memset(block + checksum_offset, 0, sizeof(tmp.checksum));
    }

    uint32_t s = crc32(block, BS);
    tmp.checksum = htole32(s);
    memcpy(sb, &tmp, sizeof(tmp));
    return s;
}

void inode_crc_finalize(inode_t* ino){
    inode_t tmp = *ino;
    tmp.mode = htole16(tmp.mode);
    tmp.links = htole16(tmp.links);
    tmp.uid = htole32(tmp.uid);
    tmp.gid = htole32(tmp.gid);
    tmp.size_bytes = htole64(tmp.size_bytes);
    tmp.atime = htole64(tmp.atime);
    tmp.mtime = htole64(tmp.mtime);
    tmp.ctime = htole64(tmp.ctime);
    for (int i = 0; i < 12; i++) tmp.direct[i] = htole32(tmp.direct[i]);
    tmp.reserved_0 = htole32(tmp.reserved_0);
    tmp.reserved_1 = htole32(tmp.reserved_1);
    tmp.reserved_2 = htole32(tmp.reserved_2);
    tmp.proj_id = htole32(tmp.proj_id);
    tmp.uid16_gid16 = htole32(tmp.uid16_gid16);
    tmp.xattr_ptr = htole64(tmp.xattr_ptr);
    tmp.inode_crc = 0;

    uint8_t buf[INODE_SIZE];
    memset(buf, 0, INODE_SIZE);
    memcpy(buf, &tmp, sizeof(tmp));
    uint32_t c = crc32(buf, INODE_SIZE - 8);
    ino->inode_crc = (uint64_t)c;
}

void dirent_checksum_finalize(dirent64_t* de) {
    const uint8_t* p = (const uint8_t*)de;
    uint8_t x = 0;
    for (int i = 0; i < 63; i++) x ^= p[i];
    de->checksum = x;
}

void inode_to_le(inode_t *ino) {
    ino->mode = htole16(ino->mode);
    ino->links = htole16(ino->links);
    ino->uid = htole32(ino->uid);
    ino->gid = htole32(ino->gid);
    ino->size_bytes = htole64(ino->size_bytes);
    ino->atime = htole64(ino->atime);
    ino->mtime = htole64(ino->mtime);
    ino->ctime = htole64(ino->ctime);
    for (int i = 0; i < 12; i++) ino->direct[i] = htole32(ino->direct[i]);
    ino->reserved_0 = htole32(ino->reserved_0);
    ino->reserved_1 = htole32(ino->reserved_1);
    ino->reserved_2 = htole32(ino->reserved_2);
    ino->proj_id = htole32(ino->proj_id);
    ino->uid16_gid16 = htole32(ino->uid16_gid16);
    ino->xattr_ptr = htole64(ino->xattr_ptr);
    ino->inode_crc = htole64(ino->inode_crc);
}
void dirent_to_le(dirent64_t *de) {
    de->inode_no = htole32(de->inode_no);
}

//main program
int main(int argc, char *argv[]) {
    crc32_init();

    // parse CLI arguments
    char *image_name = NULL;
    uint64_t size_kib = 0;
    uint64_t inode_count = 0;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--image") == 0 && i + 1 < argc) {
            image_name = argv[++i];
        } else if (strcmp(argv[i], "--size-kib") == 0 && i + 1 < argc) {
            size_kib = (uint64_t)atoi(argv[++i]);
        } else if (strcmp(argv[i], "--inodes") == 0 && i + 1 < argc) {
            inode_count = (uint64_t)atoi(argv[++i]);
        }
    }
    if (!image_name || size_kib == 0 || inode_count == 0) {
        fprintf(stderr, "Usage: mkfs_builder --image <name> --size-kib <size> --inodes <count>\n");
        return 1;
    }

    // validate
    if (size_kib < 180 || size_kib > 4096 || (size_kib % 4 != 0)) return 1;
    if (inode_count < 128 || inode_count > 512) 
        return 1;

// file system layout
    uint64_t total_blocks = (size_kib * 1024ULL) / BS;
    time_t current_time = time(NULL);
    superblock_t sb = {0};
    sb.magic = MAGIC_NUMBER;
    sb.version = 1;
    sb.block_size = BS;
    sb.total_blocks = total_blocks;
    sb.inode_count = inode_count;
    sb.root_inode = ROOT_INO;
    sb.mtime_epoch = (uint64_t)current_time;
    sb.inode_bitmap_start = 1;
    sb.inode_bitmap_blocks = 1;
    sb.data_bitmap_start = 2;
    sb.data_bitmap_blocks = 1;
    sb.inode_table_start = 3;
    uint64_t inodes_per_block = BS / INODE_SIZE;
    sb.inode_table_blocks = (inode_count + inodes_per_block - 1) / inodes_per_block;
    sb.data_region_start = sb.inode_table_start + sb.inode_table_blocks;
    sb.data_region_blocks = total_blocks - sb.data_region_start;
    superblock_crc_finalize(&sb);

    //make img 
    FILE *img = fopen(image_name, "wb+");
    if (!img) 
        return 1;

    //superblock
    uint8_t block_zero[BS]; 
    memset(block_zero, 0, BS);
    uint8_t sb_block[BS]; 
    memset(sb_block, 0, BS);
    memcpy(sb_block, &sb, sizeof(sb));
    fwrite(sb_block, 1, BS, img);

    
    uint8_t inode_bitmap[BS]; 
    memset(inode_bitmap, 0, BS);
    inode_bitmap[0] = 0x01;
    fwrite(inode_bitmap, 1, BS, img);

    
    uint8_t data_bitmap[BS]; 
    memset(data_bitmap, 0, BS);
    data_bitmap[0] = 0x01;
    fwrite(data_bitmap, 1, BS, img);

    
    fseek(img, (long)(sb.inode_table_start * BS), SEEK_SET);
    inode_t root_inode; memset(&root_inode, 0, sizeof(root_inode));
    root_inode.mode = 0x4000;
    root_inode.links = 2;
    root_inode.size_bytes = BS;
    root_inode.atime = root_inode.mtime = root_inode.ctime = (uint64_t)current_time;
    root_inode.direct[0] = (uint32_t)sb.data_region_start;
    root_inode.proj_id = GROUP_ID;
    inode_crc_finalize(&root_inode);
    inode_to_le(&root_inode);
    fwrite(&root_inode, 1, sizeof(root_inode), img);

    inode_t empty_inode; 
    memset(&empty_inode, 0, sizeof(empty_inode));

    inode_to_le(&empty_inode);
    for (uint64_t i = 1; i < inode_count; i++) {
        fwrite(&empty_inode, 1, sizeof(empty_inode), img);
    }

    //directory entries for root
    fseek(img, (long)(sb.data_region_start * BS), SEEK_SET);
    dirent64_t dot_entry, dotdot_entry;
    memset(&dot_entry, 0, sizeof(dot_entry));
    dot_entry.inode_no = 1; dot_entry.type = 2; strncpy(dot_entry.name, ".", sizeof(dot_entry.name)-1);
    dirent_checksum_finalize(&dot_entry);
    memset(&dotdot_entry, 0, sizeof(dotdot_entry));

    dotdot_entry.inode_no = 1; dotdot_entry.type = 2; strncpy(dotdot_entry.name, "..", sizeof(dotdot_entry.name)-1);
    dirent_checksum_finalize(&dotdot_entry);
    dirent_to_le(&dot_entry); 
    dirent_to_le(&dotdot_entry);

    fwrite(&dot_entry, 1, sizeof(dot_entry), img);
    fwrite(&dotdot_entry, 1, sizeof(dotdot_entry), img);
    size_t dir_used = 2 * sizeof(dirent64_t);
    size_t dir_pad = BS - dir_used;
    fwrite(block_zero, 1, dir_pad, img);

    //padding
    uint64_t total_size = total_blocks * (uint64_t)BS;
    long curpos = ftell(img);
    uint64_t current_pos = (uint64_t)curpos;
    while (current_pos < total_size) {
        uint64_t remain = total_size - current_pos;
        size_t to_write = (remain >= BS) ? BS : (size_t)remain;
        fwrite(block_zero, 1, to_write, img);
        current_pos += to_write;
    }

    fclose(img);
    printf("Successfully created filesystem image: %s\n", image_name);
    return 0;
}
