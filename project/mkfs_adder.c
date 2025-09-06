#define _FILE_OFFSET_BITS 64
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <getopt.h>

#define BS 4096u
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
    printf("Usage: mkfs_adder --input <input.img> --output <output.img> --file <filename>\n");
    exit(1);
}

// Function to find free inode using first-fit
int find_free_inode(uint8_t *bitmap, uint64_t inode_count) {
    for (uint64_t i = 0; i < inode_count; i++) {
        if (!(bitmap[i / 8] & (1 << (i % 8)))) {
            return i + 1; // 1-indexed
        }
    }
    return -1;
}

// Function to find free data block using first-fit
int find_free_data_block(uint8_t *bitmap, uint64_t data_blocks) {
    for (uint64_t i = 0; i < data_blocks; i++) {
        if (!(bitmap[i / 8] & (1 << (i % 8)))) {
            return i;
        }
    }
    return -1;
}

int main(int argc, char *argv[]) {
    crc32_init();
    
    // Parse command line arguments
    char *input_name = NULL;
    char *output_name = NULL;
    char *filename = NULL;
    
    static struct option long_options[] = {
        {"input", required_argument, 0, 'i'},
        {"output", required_argument, 0, 'o'},
        {"file", required_argument, 0, 'f'},
        {0, 0, 0, 0}
    };
    
    int opt;
    while ((opt = getopt_long(argc, argv, "i:o:f:", long_options, NULL)) != -1) {
        switch (opt) {
            case 'i': input_name = optarg; break;
            case 'o': output_name = optarg; break;
            case 'f': filename = optarg; break;
            default: usage();
        }
    }
    
    if (!input_name || !output_name || !filename) {
        usage();
    }
    
    // Open input file
    FILE *fp_in = fopen(input_name, "rb+");
    if (!fp_in) {
        perror("Failed to open input image");
        return 1;
    }
    
    // Read superblock
    superblock_t sb;
    fread(&sb, sizeof(sb), 1, fp_in);
    
    // Verify magic number
    if (sb.magic != 0x4D565346) {
        fprintf(stderr, "Error: Not a valid MiniVSFS image\n");
        fclose(fp_in);
        return 1;
    }
    
    // Read bitmaps
    uint8_t *inode_bitmap = malloc(BS);
    uint8_t *data_bitmap = malloc(BS);
    
    fseek(fp_in, sb.inode_bitmap_start * BS, SEEK_SET);
    fread(inode_bitmap, BS, 1, fp_in);
    
    fseek(fp_in, sb.data_bitmap_start * BS, SEEK_SET);
    fread(data_bitmap, BS, 1, fp_in);
    
    // Find free inode
    int free_inode = find_free_inode(inode_bitmap, sb.inode_count);
    if (free_inode == -1) {
        fprintf(stderr, "Error: No free inodes available\n");
        fclose(fp_in);
        free(inode_bitmap);
        free(data_bitmap);
        return 1;
    }
    
    // Open the file to be added
    FILE *file_fp = fopen(filename, "rb");
    if (!file_fp) {
        perror("Failed to open input file");
        fclose(fp_in);
        free(inode_bitmap);
        free(data_bitmap);
        return 1;
    }
    
    // Get file size
    fseek(file_fp, 0, SEEK_END);
    uint64_t file_size = ftell(file_fp);
    fseek(file_fp, 0, SEEK_SET);
    
    // Calculate required blocks
    uint64_t blocks_needed = (file_size + BS - 1) / BS;
    if (blocks_needed > DIRECT_MAX) {
        fprintf(stderr, "Error: File too large (max %d blocks supported)\n", DIRECT_MAX);
        fclose(fp_in);
        fclose(file_fp);
        free(inode_bitmap);
        free(data_bitmap);
        return 1;
    }
    
    // Find free data blocks
    uint32_t data_blocks[DIRECT_MAX] = {0};
    for (int i = 0; i < blocks_needed; i++) {
        int free_block = find_free_data_block(data_bitmap, sb.data_region_blocks);
        if (free_block == -1) {
            fprintf(stderr, "Error: Not enough free data blocks\n");
            fclose(fp_in);
            fclose(file_fp);
            free(inode_bitmap);
            free(data_bitmap);
            return 1;
        }
        data_blocks[i] = sb.data_region_start + free_block;
        data_bitmap[free_block / 8] |= (1 << (free_block % 8));
    }
    
    // Create file inode
    time_t now = time(NULL);
    inode_t file_inode = {
        .mode = 0x8000, // regular file
        .links = 1,
        .uid = 0,
        .gid = 0,
        .size_bytes = file_size,
        .atime = now,
        .mtime = now,
        .ctime = now,
        .direct = {0},
        .reserved_0 = 0,
        .reserved_1 = 0,
        .reserved_2 = 0,
        .proj_id = 1234,
        .uid16_gid16 = 0,
        .xattr_ptr = 0
    };
    memcpy(file_inode.direct, data_blocks, blocks_needed * sizeof(uint32_t));
    inode_crc_finalize(&file_inode);
    
    // Read root inode
    inode_t root_inode;
    fseek(fp_in, sb.inode_table_start * BS + (ROOT_INO - 1) * INODE_SIZE, SEEK_SET);
    fread(&root_inode, sizeof(root_inode), 1, fp_in);
    
    // Read root directory entries
    fseek(fp_in, root_inode.direct[0] * BS, SEEK_SET);
    uint64_t entries_per_block = BS / sizeof(dirent64_t);
    dirent64_t *entries = malloc(BS);
    fread(entries, BS, 1, fp_in);
    
    // Find free directory entry
    int free_entry = -1;
    for (int i = 0; i < entries_per_block; i++) {
        if (entries[i].inode_no == 0) {
            free_entry = i;
            break;
        }
    }
    
    if (free_entry == -1) {
        fprintf(stderr, "Error: No free directory entries in root\n");
        fclose(fp_in);
        fclose(file_fp);
        free(inode_bitmap);
        free(data_bitmap);
        free(entries);
        return 1;
    }
    
    // Create directory entry
    dirent64_t new_entry = {
        .inode_no = free_inode,
        .type = 1, // file
        .name = ""
    };
    strncpy(new_entry.name, filename, sizeof(new_entry.name) - 1);
    dirent_checksum_finalize(&new_entry);
    
    entries[free_entry] = new_entry;
    
    // Update root directory size and link count
    root_inode.size_bytes += sizeof(dirent64_t);
    root_inode.links += 1;
    root_inode.mtime = now;
    inode_crc_finalize(&root_inode);
    
    // Write file data to data blocks
    for (int i = 0; i < blocks_needed; i++) {
        fseek(fp_in, data_blocks[i] * BS, SEEK_SET);
        uint8_t buffer[BS] = {0};
        size_t bytes_read = fread(buffer, 1, (i == blocks_needed - 1) ? file_size % BS : BS, file_fp);
        fwrite(buffer, BS, 1, fp_in);
    }
    
    // Write updated structures back to image
    fseek(fp_in, sb.inode_bitmap_start * BS, SEEK_SET);
    fwrite(inode_bitmap, BS, 1, fp_in);
    
    fseek(fp_in, sb.data_bitmap_start * BS, SEEK_SET);
    fwrite(data_bitmap, BS, 1, fp_in);
    
    fseek(fp_in, sb.inode_table_start * BS + (free_inode - 1) * INODE_SIZE, SEEK_SET);
    fwrite(&file_inode, sizeof(file_inode), 1, fp_in);
    
    fseek(fp_in, sb.inode_table_start * BS + (ROOT_INO - 1) * INODE_SIZE, SEEK_SET);
    fwrite(&root_inode, sizeof(root_inode), 1, fp_in);
    
    fseek(fp_in, root_inode.direct[0] * BS, SEEK_SET);
    fwrite(entries, BS, 1, fp_in);
    
    // Create output file by copying input
    fclose(fp_in);
    fclose(file_fp);
    
    // Copy input to output
    FILE *fp_out = fopen(output_name, "wb");
    fp_in = fopen(input_name, "rb");
    if (!fp_out || !fp_in) {
        perror("Failed to create output file");
        return 1;
    }
    
    fseek(fp_in, 0, SEEK_END);
    long size = ftell(fp_in);
    fseek(fp_in, 0, SEEK_SET);
    
    uint8_t *buffer = malloc(size);
    fread(buffer, size, 1, fp_in);
    fwrite(buffer, size, 1, fp_out);
    
    fclose(fp_in);
    fclose(fp_out);
    free(buffer);
    free(inode_bitmap);
    free(data_bitmap);
    free(entries);
    
    printf("File %s added to file system as inode %d\n", filename, free_inode);
    printf("Output image: %s\n", output_name);
    
    return 0;
}