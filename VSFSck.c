#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

#define BLOCK_BYTES 4096
#define NUM_BLOCKS 64
#define SIZE_OF_INODE 256
#define VSFS_MAGIC 0xD34D
#define INODES_TOTAL (5 * BLOCK_BYTES / SIZE_OF_INODE)
#define DISK_IMG_SIZE (NUM_BLOCKS * BLOCK_BYTES)

typedef struct {
    uint16_t id;
    uint32_t blk_sz;
    uint32_t blk_count;
    uint32_t ibmp_block;
    uint32_t dbmp_block;
    uint32_t inode_start;
    uint32_t data_start;
    uint32_t inode_sz;
    uint32_t inode_total;
    uint8_t pad[4058];
} FSHeader;

typedef struct {
    uint32_t perm;
    uint32_t uid;
    uint32_t gid;
    uint32_t filesize;
    uint32_t last_access;
    uint32_t created_at;
    uint32_t last_mod;
    uint32_t deleted_at;
    uint32_t link_count;
    uint32_t block_total;
    uint32_t direct[1];
    uint32_t indirect_1;
    uint32_t indirect_2;
    uint32_t indirect_3;
    uint8_t reserved[156];
} FSInode;

typedef struct {
    int file;
    void *disk;
    size_t fsize;
    FSHeader *header;
    uint8_t *ibitmap;
    uint8_t *dbitmap;
    FSInode *inode_table;
} VFS;

int validate_header(VFS *fs) {
    int issues = 0;
    FSHeader *h = fs->header;

    if (h->id != VSFS_MAGIC) {
        printf("[ERROR] Magic ID mismatch: expected 0x%X, found 0x%X\n", VSFS_MAGIC, h->id);
        issues++;
    }
    if (h->blk_sz != BLOCK_BYTES) {
        printf("[ERROR] Block size mismatch: expected %d, got %d\n", BLOCK_BYTES, h->blk_sz);
        issues++;
    }
    if (h->blk_count != NUM_BLOCKS) {
        printf("[ERROR] Block count mismatch: expected %d, got %d\n", NUM_BLOCKS, h->blk_count);
        issues++;
    }
    if (h->ibmp_block != 1 || h->dbmp_block != 2 || h->inode_start != 3 || h->data_start != 8) {
        printf("[ERROR] Block mapping invalid\n");
        issues++;
    }
    if (h->inode_sz != SIZE_OF_INODE) {
        printf("[ERROR] Inode size mismatch\n");
        issues++;
    }
    if (h->inode_total != INODES_TOTAL) {
        printf("[ERROR] Inode count mismatch\n");
        issues++;
    }
    return issues;
}

int check_data_blocks(VFS *fs, uint32_t *block_refs) {
    int faults = 0;
    uint8_t seen[BLOCK_BYTES] = {0};

    for (int i = 0; i < INODES_TOTAL; i++) {
        FSInode *inode = &fs->inode_table[i];
        if (inode->link_count && inode->deleted_at == 0) {
            uint32_t blk = inode->direct[0];
            if (blk < fs->header->data_start || blk >= NUM_BLOCKS) continue;
            seen[(blk - fs->header->data_start) / 8] |= (1 << ((blk - fs->header->data_start) % 8));
            block_refs[blk]++;
        }
    }

    for (int i = 0; i < NUM_BLOCKS - fs->header->data_start; i++) {
        int actual = (fs->dbitmap[i / 8] >> (i % 8)) & 1;
        int tracked = (seen[i / 8] >> (i % 8)) & 1;

        if (actual && !tracked) {
            printf("[WARNING] Data block %d marked used but unreferenced\n", i + fs->header->data_start);
            faults++;
        }
        if (!actual && tracked) {
            printf("[FIXED] Data block %d not marked used but referenced\n", i + fs->header->data_start);
            fs->dbitmap[i / 8] |= (1 << (i % 8));
            faults++;
        }
    }
    return faults;
}

int check_inode_bitmap(VFS *fs) {
    int mismatches = 0;
    uint8_t expected[BLOCK_BYTES] = {0};

    for (int i = 0; i < INODES_TOTAL; i++) {
        if (fs->inode_table[i].link_count && fs->inode_table[i].deleted_at == 0) {
            expected[i / 8] |= (1 << (i % 8));
        }
    }

    for (int i = 0; i < INODES_TOTAL; i++) {
        int on_disk = (fs->ibitmap[i / 8] >> (i % 8)) & 1;
        int needed = (expected[i / 8] >> (i % 8)) & 1;

        if (on_disk != needed) {
            printf("[CORRECTED] Inode %d bitmap mismatch\n", i);
            if (needed) fs->ibitmap[i / 8] |= (1 << (i % 8));
            else fs->ibitmap[i / 8] &= ~(1 << (i % 8));
            mismatches++;
        }
    }
    return mismatches;
}

int detect_shared_blocks(VFS *fs, uint32_t *block_refs) {
    int count = 0;
    for (int i = fs->header->data_start; i < NUM_BLOCKS; i++) {
        if (block_refs[i] > 1) {
            printf("[ERROR] Block %d shared across %d inodes\n", i, block_refs[i]);
            count++;
        }
    }
    return count;
}

int detect_invalid_blocks(VFS *fs) {
    int issues = 0;
    for (int i = 0; i < INODES_TOTAL; i++) {
        FSInode *node = &fs->inode_table[i];
        if (node->link_count && node->deleted_at == 0) {
            uint32_t blk = node->direct[0];
            if (blk >= NUM_BLOCKS || blk < fs->header->data_start) {
                printf("[INVALID] Inode %d points to bad block %d\n", i, blk);
                node->direct[0] = 0;
                issues++;
            }
        }
    }
    return issues;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filesystem.img>\n", argv[0]);
        return 1;
    }

    VFS fs;
    fs.file = open(argv[1], O_RDWR);
    if (fs.file < 0) {
        perror("Open failed");
        return 1;
    }

    struct stat st;
    fstat(fs.file, &st);
    fs.fsize = st.st_size;
    if (fs.fsize < DISK_IMG_SIZE) {
        fprintf(stderr, "Image too small (%zu bytes)\n", fs.fsize);
        close(fs.file);
        return 1;
    }

    fs.disk = mmap(NULL, fs.fsize, PROT_READ | PROT_WRITE, MAP_SHARED, fs.file, 0);
    if (fs.disk == MAP_FAILED) {
        perror("mmap error");
        close(fs.file);
        return 1;
    }

    fs.header = (FSHeader *)fs.disk;
    fs.ibitmap = (uint8_t *)(fs.disk + BLOCK_BYTES);
    fs.dbitmap = (uint8_t *)(fs.disk + 2 * BLOCK_BYTES);
    fs.inode_table = (FSInode *)(fs.disk + 3 * BLOCK_BYTES);

    uint32_t block_tracker[NUM_BLOCKS] = {0};
    int total_issues = 0;

    printf("Running superblock check...\n");
    total_issues += validate_header(&fs);

    printf("Verifying data bitmap usage...\n");
    total_issues += check_data_blocks(&fs, block_tracker);

    printf("Validating inode bitmap integrity...\n");
    total_issues += check_inode_bitmap(&fs);

    printf("Scanning for duplicate block usage...\n");
    total_issues += detect_shared_blocks(&fs, block_tracker);

    printf("Identifying bad block references...\n");
    total_issues += detect_invalid_blocks(&fs);

    printf("Total Issues Addressed: %d\n", total_issues);

    msync(fs.disk, fs.fsize, MS_SYNC);
    munmap(fs.disk, fs.fsize);
    close(fs.file);
    return 0;
}
