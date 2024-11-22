#include "fs_file.h"

#ifndef FS_FAT_H
#define FS_FAT_H

typedef struct fs_fat_struct {
    char* name;
    int num_blocks;
    int block_size;
    int fat_size;
    int num_free_blocks;
    fs_file* files_head;
    fs_file* files_tail;
    uint16_t* fat_table;
} fs_fat;

// makefs: make a fs on disk
int create_fs(char* fs_name, int num_blocks, int block_size);
// mount: load the fat table in memory, and create the linked list of fs_files
fs_fat* load_fat(char* fs_name);
// save the updated lists of directory files to disk (no free the malloc)
int save_list_fs_files_to_disk(fs_fat* fat);
// delete fs on disk
int delete_fs(char* fs_name);

#endif