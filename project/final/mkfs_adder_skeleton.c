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
_Static_assert(sizeof(superblock_t) == 116, "superblock size mismatch");

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
_Static_assert(sizeof(inode_t)==INODE_SIZE, "inode size mismatch");

#pragma pack(push,1)
typedef struct {
    uint32_t inode_no;
    uint8_t  type;
    char     name[58];
    uint8_t  checksum;
} dirent64_t;
#pragma pack(pop)
_Static_assert(sizeof(dirent64_t)==64, "dirent size mismatch");

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

    size_t off = offsetof(superblock_t, checksum);
    memset(block + off, 0, 4);

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
    for(int i=0;i<DIRECT_MAX;i++) tmp.direct[i] = htole32(tmp.direct[i]);
    tmp.reserved_0 = htole32(tmp.reserved_0);
    tmp.reserved_1 = htole32(tmp.reserved_1);
    tmp.reserved_2 = htole32(tmp.reserved_2);
    tmp.proj_id = htole32(tmp.proj_id);
    tmp.uid16_gid16 = htole32(tmp.uid16_gid16);
    tmp.xattr_ptr = htole64(tmp.xattr_ptr);
    tmp.inode_crc = 0;

    uint8_t buf[INODE_SIZE];
    memcpy(buf, &tmp, INODE_SIZE);

    uint32_t c = crc32(buf, INODE_SIZE - 8);
    ino->inode_crc = (uint64_t)c;
}

void dirent_checksum_finalize(dirent64_t* de) {
    const uint8_t* p = (const uint8_t*)de;
    uint8_t x=0;
    for (int i=0;i<63;i++) x ^= p[i];
    de->checksum = x;
}

void superblock_from_le(superblock_t *sb) {
    sb->magic = le32toh(sb->magic);
    sb->version = le32toh(sb->version);
    sb->block_size = le32toh(sb->block_size);
    sb->total_blocks = le64toh(sb->total_blocks);
    sb->inode_count = le64toh(sb->inode_count);
    sb->inode_bitmap_start = le64toh(sb->inode_bitmap_start);
    sb->inode_bitmap_blocks = le64toh(sb->inode_bitmap_blocks);
    sb->data_bitmap_start = le64toh(sb->data_bitmap_start);
    sb->data_bitmap_blocks = le64toh(sb->data_bitmap_blocks);
    sb->inode_table_start = le64toh(sb->inode_table_start);
    sb->inode_table_blocks = le64toh(sb->inode_table_blocks);
    sb->data_region_start = le64toh(sb->data_region_start);
    sb->data_region_blocks = le64toh(sb->data_region_blocks);
    sb->root_inode = le64toh(sb->root_inode);
    sb->mtime_epoch = le64toh(sb->mtime_epoch);
    sb->flags = le32toh(sb->flags);
    sb->checksum = le32toh(sb->checksum);
}

void inode_from_le(inode_t *ino) {
    ino->mode = le16toh(ino->mode);
    ino->links = le16toh(ino->links);
    ino->uid = le32toh(ino->uid);
    ino->gid = le32toh(ino->gid);
    ino->size_bytes = le64toh(ino->size_bytes);
    ino->atime = le64toh(ino->atime);
    ino->mtime = le64toh(ino->mtime);
    ino->ctime = le64toh(ino->ctime);
    for(int i=0;i<DIRECT_MAX;i++) ino->direct[i] = le32toh(ino->direct[i]);
    ino->reserved_0 = le32toh(ino->reserved_0);
    ino->reserved_1 = le32toh(ino->reserved_1);
    ino->reserved_2 = le32toh(ino->reserved_2);
    ino->proj_id = le32toh(ino->proj_id);
    ino->uid16_gid16 = le32toh(ino->uid16_gid16);
    ino->xattr_ptr = le64toh(ino->xattr_ptr);
    ino->inode_crc = le64toh(ino->inode_crc);
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
    for(int i=0;i<DIRECT_MAX;i++) ino->direct[i] = htole32(ino->direct[i]);
    ino->reserved_0 = htole32(ino->reserved_0);
    ino->reserved_1 = htole32(ino->reserved_1);
    ino->reserved_2 = htole32(ino->reserved_2);
    ino->proj_id = htole32(ino->proj_id);
    ino->uid16_gid16 = htole32(ino->uid16_gid16);
    ino->xattr_ptr = htole64(ino->xattr_ptr);
    ino->inode_crc = htole64(ino->inode_crc);
}

void dirent_from_le(dirent64_t *de){ de->inode_no = le32toh(de->inode_no); }
void dirent_to_le(dirent64_t *de){ de->inode_no = htole32(de->inode_no); }





//  Main program
int main(int argc, char *argv[]) {
    crc32_init();

    char *input_name=NULL,*output_name=NULL,*file_name=NULL;
    for(int i=1;i<argc;i++){
        if(strcmp(argv[i],"--input")==0 && i+1<argc) 
            input_name=argv[++i];
        else if(strcmp(argv[i],"--output")==0 && i+1<argc) 
            output_name=argv[++i];
        else if(strcmp(argv[i],"--file")==0 && i+1<argc) 
            file_name=argv[++i];
    }
    if(!input_name||!output_name||!file_name){
        fprintf(stderr,"Usage: mkfs_adder --input <in.img> --output <out.img> --file <filename>\n");
        return 1;
    }

    // copy input to output
    FILE *in=fopen(input_name,"rb");
    if(!in){ 
        perror("open input"); 
        return 1; 
    }
    FILE *out=fopen(output_name,"wb");
    if(!out){ 
        perror("create output"); 
        fclose(in); 
        return 1; 
    }
    uint8_t buf[BS];
    size_t n;
    while((n=fread(buf,1,BS,in))>0){
        if(fwrite(buf,1,n,out)!=n){ 
            perror("copy"); 
            fclose(in); 
            fclose(out); 
            return 1; 
        }
    }
    fclose(in); 
    fclose(out);

    // open output 
    FILE *fs=fopen(output_name,"rb+");
    if(!fs){ perror("reopen output"); 
        return 1; 
    }

    // superblock read valid
    superblock_t sb;
    if(fread(&sb,sizeof(sb),1,fs)!=1){ 
        fprintf(stderr,"read sb failed\n"); 
        fclose(fs); 
        return 1; 
    }
    superblock_from_le(&sb);
    if(sb.magic!=MAGIC_NUMBER){ 
        fprintf(stderr,"Not a MiniVSFS image\n"); 
        fclose(fs); 
        return 1; 
    }

    //open file to add
    FILE *f=fopen(file_name,"rb");
    if(!f){ 
        perror("open file"); 
        fclose(fs); 
        return 1; 
    }
    fseek(f,0,SEEK_END); 
    uint64_t file_size=ftell(f); 
    fseek(f,0,SEEK_SET);
    if(file_size > 12ULL*BS){ 
        fprintf(stderr,"File too large\n"); 
        fclose(f); 
        fclose(fs); 
        return 1; 
    }

    time_t now=time(NULL);

    // bitmaps from filesystem
    uint8_t inode_bm[BS], data_bm[BS];
    fseek(fs, sb.inode_bitmap_start*BS, SEEK_SET); 
    fread(inode_bm,BS,1,fs);
    fseek(fs, sb.data_bitmap_start*BS, SEEK_SET); 
    fread(data_bm,BS,1,fs);

    // get free inode in bitmap
    int free_ino=-1;
    for(int i=0;i<sb.inode_count;i++){
        int byte=i/8, bit=i%8;
        if(!(inode_bm[byte]&(1<<bit))){
            free_ino=i+1;
            inode_bm[byte]|=(1<<bit);
            break;
        }
    }
    if(free_ino==-1){ 
        fprintf(stderr,"No free inode\n"); 
        fclose(f); 
        fclose(fs); 
        return 1; 
    }

    //Datablock for file
    int blocks_needed=(file_size+BS-1)/BS;
    uint32_t data_blocks[12]={0}; 
    int got=0;
    for(uint64_t i=0;i<sb.data_region_blocks && got<blocks_needed;i++){
        int byte=i/8,bit=i%8;
        if(!(data_bm[byte]&(1<<bit))){
            data_bm[byte]|=(1<<bit);
            data_blocks[got++]=sb.data_region_start+i;
        }
    }
    if(got<blocks_needed){ 
        fprintf(stderr,"Not enough data blocks\n"); 
        fclose(f); 
        fclose(fs); 
        return 1; 
    }

    //make and write file inode
    inode_t finode={0};
    finode.mode=0x8000; finode.links=1;
    finode.size_bytes=file_size;
    finode.atime=finode.mtime=finode.ctime=now;
    memcpy(finode.direct,data_blocks,blocks_needed*sizeof(uint32_t));
    finode.proj_id=GROUP_ID;
    inode_crc_finalize(&finode);
    inode_to_le(&finode);

    uint64_t ino_offset=sb.inode_table_start*BS + (free_ino-1)*INODE_SIZE;
    fseek(fs,ino_offset,SEEK_SET);
    fwrite(&finode,sizeof(finode),1,fs);

    // write file and allocatee blocks
    for(int i=0;i<blocks_needed;i++){
        size_t r=fread(buf,1,BS,f);
        if(r==0 && !feof(f)){ 
            fprintf(stderr,"Read file error\n"); 
            fclose(f); 
            fclose(fs); 
            return 1; 
        }
        if(r<BS) 
            memset(buf+r,0,BS-r);
        fseek(fs,data_blocks[i]*BS,SEEK_SET);
        fwrite(buf,1,BS,fs);
    }
    fclose(f);

    //directory update for new file
    dirent64_t entries[BS/sizeof(dirent64_t)];
    fseek(fs,sb.data_region_start*BS,SEEK_SET);
    fread(entries,sizeof(entries),1,fs);
    for(int i=0;i<BS/sizeof(dirent64_t);i++) 
        dirent_from_le(&entries[i]);
    int slot=-1;
    for(int i=0;i<BS/sizeof(dirent64_t);i++) 
        if(entries[i].inode_no==0){ 
            slot=i; break; 
        }
    if(slot==-1){ 
        fprintf(stderr,"Root dir full\n"); 
        fclose(fs); return 1; 
    }
    dirent64_t ne={0};
    ne.inode_no=free_ino; 
    ne.type=1;

    const char *base=strrchr(file_name,'/'); 
    base=base?base+1:file_name;

    strncpy(ne.name,base,sizeof(ne.name)-1);
    dirent_checksum_finalize(&ne);
    dirent_to_le(&ne);
    entries[slot]=ne;
    fseek(fs,sb.data_region_start*BS,SEEK_SET);
    fwrite(entries,sizeof(entries),1,fs);

    //metadata update for root inode
    inode_t root;
    fseek(fs,sb.inode_table_start*BS,SEEK_SET);
    fread(&root,sizeof(root),1,fs);
    inode_from_le(&root);
    root.mtime=root.ctime=now;
    
    root.links++;
    
    inode_crc_finalize(&root);
    inode_to_le(&root);
    fseek(fs,sb.inode_table_start*BS,SEEK_SET);
    fwrite(&root,sizeof(root),1,fs);

    // write updated bitmaps back to filesystem
    fseek(fs,sb.inode_bitmap_start*BS,SEEK_SET); 
    fwrite(inode_bm,BS,1,fs);
    fseek(fs,sb.data_bitmap_start*BS,SEEK_SET); 
    fwrite(data_bm,BS,1,fs);

    // update superblock timestamp and checksum
    sb.mtime_epoch=now;
    superblock_crc_finalize(&sb);
    uint8_t sb_block[BS]={0};
    memcpy(sb_block,&sb,sizeof(sb));
    fseek(fs,0,SEEK_SET);
    fwrite(sb_block,1,BS,fs);

    fclose(fs);
    printf("Added file '%s' -> inode %d, size %llu bytes\n", file_name, free_ino, (unsigned long long)file_size);
    return 0;
}