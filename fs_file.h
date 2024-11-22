#include <stdint.h>
#include <time.h>

#ifndef FS_FILE_H
#define FS_FILE_H

/**
* @brief struct for file metadata in a directory file
*/
typedef struct fs_file_struct {
    char name[32];
    uint32_t size;
    uint16_t first_block;
    uint8_t type;
    uint8_t perm;
    time_t mtime;
    struct fs_file_struct* next;
} fs_file;

fs_file* add_fs_file(fs_file* tail, uint8_t* buf);

void print_fs_file_info(fs_file* file);

#endif