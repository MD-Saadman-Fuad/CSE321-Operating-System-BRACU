// Build: gcc -O2 -std=c17 -Wall -Wextra mkfs_builder.c -o mkfs_builder
#define _FILE_OFFSET_BITS 64
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>
#include <errno.h>
#include <time.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>

#define BS 4096u               // block size
#define INODE_SIZE 128u
#define ROOT_INO 1u
#define DIRECT_MAX 12

// ... [struct definitions from above] ...
#pragma pack(push, 1)
typedef struct {
    // Superblock fields as per specification
    uint32_t magic;                 // 4 bytes: 0x4D565346
    uint32_t version;               // 4 bytes: 1
    uint32_t block_size;            // 4 bytes: 4096
    uint64_t total_blocks;          // 8 bytes
    uint64_t inode_count;           // 8 bytes
    uint64_t inode_bitmap_start;    // 8 bytes
    uint64_t inode_bitmap_blocks;   // 8 bytes
    uint64_t data_bitmap_start;     // 8 bytes
    uint64_t data_bitmap_blocks;    // 8 bytes
    uint64_t inode_table_start;     // 8 bytes
    uint64_t inode_table_blocks;    // 8 bytes
    uint64_t data_region_start;     // 8 bytes
    uint64_t data_region_blocks;    // 8 bytes
    uint64_t root_inode;            // 8 bytes: 1
    uint64_t mtime_epoch;           // 8 bytes: Build time
    uint32_t flags;                 // 4 bytes: 0
    uint32_t checksum;              // 4 bytes: crc32(superblock[0..4091])
} superblock_t;
#pragma pack(pop)
_Static_assert(sizeof(superblock_t) == 116, "superblock must fit in one block");

#pragma pack(push,1)
typedef struct {
    // Inode fields as per specification
    uint16_t mode;                  // 2 bytes
    uint16_t links;                 // 2 bytes
    uint32_t uid;                   // 4 bytes: 0
    uint32_t gid;                   // 4 bytes: 0
    uint64_t size_bytes;            // 8 bytes
    uint64_t atime;                 // 8 bytes
    uint64_t mtime;                 // 8 bytes
    uint64_t ctime;                 // 8 bytes
    uint32_t direct[12];            // 48 bytes (12 * 4)
    uint32_t reserved_0;            // 4 bytes: 0
    uint32_t reserved_1;            // 4 bytes: 0
    uint32_t reserved_2;            // 4 bytes: 0
    uint32_t proj_id;               // 4 bytes: Your group ID
    uint32_t uid16_gid16;           // 4 bytes: 0
    uint64_t xattr_ptr;             // 8 bytes: 0
    uint64_t inode_crc;             // 8 bytes: crc32 of bytes [0..119]
} inode_t;
#pragma pack(pop)
_Static_assert(sizeof(inode_t)==INODE_SIZE, "inode size mismatch");

#pragma pack(push,1)
typedef struct {
    // Directory entry structure
    uint32_t inode_no;              // 4 bytes
    uint8_t  type;                  // 1 byte: 1=file, 2=dir
    char     name[58];              // 58 bytes
    uint8_t  checksum;              // 1 byte: XOR of bytes 0..62
} dirent64_t;
#pragma pack(pop)
_Static_assert(sizeof(dirent64_t)==64, "dirent size mismatch");
// ==========================DO NOT CHANGE THIS PORTION=========================
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
    sb->checksum = 0;
    uint32_t s = crc32((void *) sb, BS - 4);
    sb->checksum = s;
    return s;
}

void inode_crc_finalize(inode_t* ino){
    uint8_t tmp[INODE_SIZE]; memcpy(tmp, ino, INODE_SIZE);
    memset(&tmp[120], 0, 8);
    uint32_t c = crc32(tmp, 120);
    ino->inode_crc = (uint64_t)c;
}

void dirent_checksum_finalize(dirent64_t* de) {
    const uint8_t* p = (const uint8_t*)de;
    uint8_t x = 0;
    for (int i = 0; i < 63; i++) x ^= p[i];
    de->checksum = x;
}
// =============================================================================

void usage() {
    printf("Usage: mkfs_builder --image <output.img> --size-kib <180..4096> --inodes <128..512>\n");
    exit(1);
}

int main(int argc, char *argv[]) {
    crc32_init();
    
    // Parse command line arguments
    char *image_name = NULL;
    uint64_t size_kib = 0;
    uint64_t inode_count = 0;
    
    static struct option long_options[] = {
        {"image", required_argument, 0, 'i'},
        {"size-kib", required_argument, 0, 's'},
        {"inodes", required_argument, 0, 'n'},
        {0, 0, 0, 0}
    };
    
    int opt;
    while ((opt = getopt_long(argc, argv, "i:s:n:", long_options, NULL)) != -1) {
        switch (opt) {
            case 'i': image_name = optarg; break;
            case 's': size_kib = atoll(optarg); break;
            case 'n': inode_count = atoll(optarg); break;
            default: usage();
        }
    }
    
    // Validate parameters
    if (!image_name || size_kib < 180 || size_kib > 4096 || inode_count < 128 || inode_count > 512) {
        usage();
    }
    
    // Calculate file system parameters
    uint64_t total_blocks = size_kib * 1024 / BS;
    uint64_t inode_table_blocks = (inode_count * INODE_SIZE + BS - 1) / BS;
    uint64_t data_region_blocks = total_blocks - 4 - inode_table_blocks; // super(1) + inode_bm(1) + data_bm(1) + inode_table
    
    // Verify we have enough space
    if (data_region_blocks < 1) {
        fprintf(stderr, "Error: Not enough space for data region\n");
        return 1;
    }
    
    // Create and initialize superblock
    superblock_t sb = {
        .magic = 0x4D565346,
        .version = 1,
        .block_size = BS,
        .total_blocks = total_blocks,
        .inode_count = inode_count,
        .inode_bitmap_start = 1,
        .inode_bitmap_blocks = 1,
        .data_bitmap_start = 2,
        .data_bitmap_blocks = 1,
        .inode_table_start = 3,
        .inode_table_blocks = inode_table_blocks,
        .data_region_start = 3 + inode_table_blocks,
        .data_region_blocks = data_region_blocks,
        .root_inode = ROOT_INO,
        .mtime_epoch = time(NULL),
        .flags = 0
    };
    superblock_crc_finalize(&sb);
    
    // Create bitmaps
    uint8_t *inode_bitmap = calloc(BS, 1);
    uint8_t *data_bitmap = calloc(BS, 1);
    
    // Allocate root inode (ino 1) and its data block
    inode_bitmap[0] = 0x03; // bits 0 and 1 set (inodes 1 and 2 allocated)
    data_bitmap[0] = 0x01;  // data block 0 allocated
    
    // Create root inode
    time_t now = time(NULL);
    inode_t root_inode = {
        .mode = 0x4000, // directory
        .links = 2,      // . and ..
        .uid = 0,
        .gid = 0,
        .size_bytes = 2 * sizeof(dirent64_t),
        .atime = now,
        .mtime = now,
        .ctime = now,
        .direct = {sb.data_region_start}, // first data block
        .reserved_0 = 0,
        .reserved_1 = 0,
        .reserved_2 = 0,
        .proj_id = 1234, // example group ID
        .uid16_gid16 = 0,
        .xattr_ptr = 0
    };
    inode_crc_finalize(&root_inode);
    
    // Create root directory entries
    dirent64_t dot_entry = {
        .inode_no = ROOT_INO,
        .type = 2, // directory
        .name = "."
    };
    dirent_checksum_finalize(&dot_entry);
    
    dirent64_t dotdot_entry = {
        .inode_no = ROOT_INO,
        .type = 2, // directory
        .name = ".."
    };
    dirent_checksum_finalize(&dotdot_entry);
    
    // Write the file system image
    FILE *fp = fopen(image_name, "wb");
    if (!fp) {
        perror("Failed to create image file");
        return 1;
    }
    
    // Write superblock
    fwrite(&sb, sizeof(sb), 1, fp);
    fseek(fp, BS, SEEK_SET); // Pad to block size
    
    // Write inode bitmap
    fwrite(inode_bitmap, BS, 1, fp);
    
    // Write data bitmap
    fwrite(data_bitmap, BS, 1, fp);
    
    // Write inode table (pad with zeros)
    fseek(fp, sb.inode_table_start * BS, SEEK_SET);
    fwrite(&root_inode, sizeof(root_inode), 1, fp);
    // Pad the rest of inode table with zeros
    uint64_t inode_table_size = sb.inode_table_blocks * BS;
    uint8_t *zeros = calloc(inode_table_size - sizeof(root_inode), 1);
    fwrite(zeros, inode_table_size - sizeof(root_inode), 1, fp);
    free(zeros);
    
    // Write data region
    fseek(fp, sb.data_region_start * BS, SEEK_SET);
    fwrite(&dot_entry, sizeof(dot_entry), 1, fp);
    fwrite(&dotdot_entry, sizeof(dotdot_entry), 1, fp);
    
    // Pad the rest of the image with zeros
    uint64_t total_size = total_blocks * BS;
    fseek(fp, total_size - 1, SEEK_SET);
    fputc(0, fp);
    
    fclose(fp);
    free(inode_bitmap);
    free(data_bitmap);
    
    printf("File system image created: %s\n", image_name);
    printf("Total blocks: %" PRIu64 ", Inodes: %" PRIu64 ", Data blocks: %" PRIu64 "\n", 
           total_blocks, inode_count, data_region_blocks);
    
    return 0;
}