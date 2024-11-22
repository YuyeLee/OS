#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include "fs_fat.h"

void test_create_fs() {
    printf("Testing create_fs...\n");
    int result = create_fs("FILE_SYSTEM", 32, 4);
    if (result == 0) {
        printf("create_fs: Success\n");
    } else {
        printf("create_fs: Failed\n");
    }
}

void test_load_fat() {
    printf("Testing load_fat...\n");
    fs_fat* fat = load_fat("FILE_SYSTEM");
    if (fat != NULL) {
        printf("load_fat: Success\n");
        printf("Block size: %d\n", fat->block_size);
        printf("Number of blocks: %d\n", fat->num_blocks);
        printf("Number of free blocks: %d\n", fat->num_free_blocks);
        // Free the allocated memory
        munmap(fat->fat_table, fat->fat_size);
        free(fat);
    } else {
        printf("load_fat: Failed\n");
    }
}

void test_save_list_fs_files_to_disk() {
    printf("Testing save_list_fs_files_to_disk...\n");
    fs_fat* fat = load_fat("FILE_SYSTEM");
    if (fat != NULL) {
        int result = save_list_fs_files_to_disk(fat);
        if (result == 0) {
            printf("save_list_fs_files_to_disk: Success\n");
        } else {
            printf("save_list_fs_files_to_disk: Failed\n");
        }
        // Free the allocated memory
        munmap(fat->fat_table, fat->fat_size);
        free(fat);
    } else {
        printf("Failed to load filesystem for saving test\n");
    }
}

int main() {
    test_create_fs();
    test_load_fat();
    test_save_list_fs_files_to_disk();
    return 0;
}