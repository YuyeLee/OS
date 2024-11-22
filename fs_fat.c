#include "fs_fat.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>


// makefs: make a fs on disk
int create_fs(char* fs_name, int num_blocks, int block_size){
    FILE* file = fopen(fs_name, "wb");
    if (file == NULL) { // check for errors opening the file
        printf("Error opening file %s\n", fs_name);
        return 1;
    }
    // hd is little endian
    // LSB is the block size
    // MSB is the number of blocks
    fwrite(&block_size, sizeof(uint8_t), 1, file);
    fwrite(&num_blocks, sizeof(uint8_t), 1, file);
    int val = 0xff;
    fwrite(&val, sizeof(uint8_t), 1, file);
    fwrite(&val, sizeof(uint8_t), 1, file);
    fclose(file);
    
    long long num_bytes_fat_table = (1 << (block_size + 8)) * num_blocks;
    // printf("%lld\n", num_bytes_fat_table);
    long long num_bytes_data = (num_bytes_fat_table / 2 - 1) * (1 << (block_size + 8));
    // printf("%lld\n", num_bytes_data);

    // the maximum possible block number of PennFAT is 2^16 -2 
    // because 0xFFFF denote the NULL pointer
    // #entries = 32 * 4096 / 2 = 65536 which means the entry[0xFFFF]is undefined
    // need to subtract the last block's byte from the data region
    if (num_blocks == 32 && block_size == 4) {
        num_bytes_data -= 4096;
    }
    //! can try using ftruncate
    off_t file_size = num_bytes_fat_table + num_bytes_data;
    int fd = open(fs_name, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        perror("Error opening file");
        return 1;
    }
    // set the size of the file and fill with zeros
    if (ftruncate(fd, file_size) == -1) {
        perror("Error setting file size");
        return 1;
    }
    close(fd);
    return 0;
}

void create_list_fs_files(fs_fat* fat) {
    // dum head
    //! remember to free
    fs_file* head = malloc(sizeof(fs_file));
    fs_file* tail = head;
    //! need to set next to NULL?
    tail->next = NULL;
    int fd;
    //! read only?
    if ((fd = open(fat->name, O_RDWR, 0644)) == -1) {
        perror("open");
        return;
    }
    // first block index of root directory is 1
    int cur_block_index = 1;
    while (cur_block_index != 0xFFFF) {
        int next_block_index = fat->fat_table[cur_block_index];
        // seek to the first block in data region
        if (lseek(fd, fat->fat_size + (cur_block_index - 1) * fat->block_size, SEEK_SET) == -1) {
            perror("lseek");
            return;
        }
        // TODO: build a linked list of files by reading the files in this block
        // there will be at most (block_size / 64) number of files in each block
        // each file is size 64 bytes
        // You can read the next 64 bytes into a buffer until buffer[0] = 0x00, indicating the end of the directory.
        // and then use add_fs_file(tail, buffer) to add the file to the list.
        break;
    }
    fat->files_head = head->next;
    free(head);
    fat->files_tail = tail;
}

// mount: load the fat table in memory
fs_fat* load_fat(char* fs_name){
    // printf("%s\n", fs_name);
    FILE* file = fopen(fs_name, "r");
    if (file == NULL) {
        printf("Error opening file.\n");
        return NULL;
    }
    uint8_t block_size_nums[2];
    fread(block_size_nums, sizeof(block_size_nums), 1, file);
    fclose(file);

    //! remember to free
    // TODO: Construct the fs_fat struct using the block_size_nums information
    // assign fat->fat_table with mmap() system call
    

    // Here below we are reading from the file system to create a list of files
    // by reading the root directory.
    create_list_fs_files(fat);

    // calculate number of free block by traversing the fat table
    fat->num_free_blocks = 0;
    for (int i = 0; i < fat->fat_size / 2; ++i) {
        if (fat->fat_table[i] == 0) {
            fat->num_free_blocks++;
        }
    }

    // TEST CODE
    // int num_blocks_data = (fat->num_blocks * fat->block_size) / 2;
    // printf("number of free blocks %d\n", fat->num_free_blocks);
    // printf("total number of blocks in data region %d\n", num_blocks_data);

    // int i = 0;
    // for (; i < fat->fat_size / 2; ++i) {
    //     // printf("%d\n", fat->fat_table[i]);
    // }
    // printf("%x\n", (fat->fat_table[0]));
    // printf("i = %d\n", i);
    
    return fat;
}
// save the updated lists of directory files to disk
int save_list_fs_files_to_disk(fs_fat* fat){
    int fd;
    //! read only?
    if ((fd = open(fat->name, O_RDWR, 0644)) == -1) {
        perror("open");
        return 1;
    }

    // get how many fs_file do we have
    fs_file* tmp = fat->files_head;
    int num_fs_files = 0;
    while (tmp) {
      ++num_fs_files;
      tmp = tmp->next;
    }

    // check how many block do we need
    int num_blocks_need = num_fs_files / (fat->block_size / 64);
    if (num_fs_files % (fat->block_size / 64) != 0) ++num_blocks_need;
    // printf("num of block needed: %d\n", num_blocks_need);

    // first block index of root directory is 1
    int cur_block_index = 1;
    int end_of_fs_files_block = 0xFFFF;
    fs_file* head = fat->files_head;
    while (1) {
        // seek to the first block in data region
        if (lseek(fd, fat->fat_size + (cur_block_index - 1) * fat->block_size, SEEK_SET) == -1) {
            perror("lseek");
            return 1;
        }
        // there will be at most (block_size / 64) number of files in each block
        for (int file_index = 0; file_index < fat->block_size / 64; ++file_index) {
            if (head == NULL) {
                // we need to add a end of the directory file 0x00 (null byte)
                // because the block is not a full block with directory files
                int val = 0x00;
                if (write(fd, &val, sizeof(uint8_t)) == -1) {
                    perror("write");
                    return 1;
                }
                break;
            }
            // write each fs_file metadata into a buf
            uint8_t buf[64];
            for (int i = 0; i < 64; i++) {
                buf[i] = 0x00;
            }
            memcpy(buf, head->name, strlen(head->name) + 1);
            memcpy((buf + 32), &head->size, sizeof(uint8_t) * 4);
            memcpy((buf + 36), &head->first_block, sizeof(uint8_t) * 2);
            memcpy((buf + 38), &head->type, sizeof(uint8_t) * 1);
            memcpy((buf + 39), &head->perm, sizeof(uint8_t) * 1);
            memcpy((buf + 40), &head->mtime, sizeof(uint8_t) * 8);

            // write the buf into fd (file system)
            if (write(fd, buf, sizeof(uint8_t) * 64) == -1) {
                perror("write");
                return 1;
            }

            // get the next fs_file in the list
            head = head->next;
        }
        // all fs_files has been written
        if (head == NULL) {
          end_of_fs_files_block = cur_block_index;
          break;
        }

        // get the next block
        int next_block_index = fat->fat_table[cur_block_index];
        if (next_block_index == 0x0000 || next_block_index == 0xFFFF) {
          // current block is the last block, find a free block from FAT table
          for (int i = 1; i < (fat->fat_size / 2); i++) {
            if (fat->fat_table[i] == 0x0000) {
              next_block_index = i;
              break;
            }
          }
          fat->fat_table[cur_block_index] = next_block_index;
          cur_block_index = next_block_index;
        } else {
          cur_block_index = next_block_index;
        }
    }

    // make all the blocks that are previous filled after the fs_files tail
    // in the fat_table set to 0x0000
    int tmp_cur_fat_index = end_of_fs_files_block;
    while (tmp_cur_fat_index != 0x0000) {
      int nextBlockIndex = fat->fat_table[tmp_cur_fat_index];
      fat->fat_table[tmp_cur_fat_index] = 0x0000;
      fat->num_free_blocks++;
      if (nextBlockIndex == 0xFFFF) {
        fat->fat_table[tmp_cur_fat_index] = 0x0000;
        break;
      }
      tmp_cur_fat_index = nextBlockIndex;
    }
    fat->fat_table[end_of_fs_files_block] = 0xFFFF;

    close(fd);

    return 0;
}
// delete
int delete_fs(char* fs_name){
   int status;
   status = remove(fs_name);
   return status;
}
