#include "fs_file.h"
#include "fs_file.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>


fs_file* add_fs_file(fs_file* tail, uint8_t* buf) {
    //! remember to free
    fs_file* new_tail = malloc(sizeof(fs_file));
    new_tail->next = NULL;
    memcpy(&(new_tail->name), buf, sizeof(uint8_t) * 32);
    memcpy(&(new_tail->size), (buf + 32), sizeof(uint8_t) * 4);
    memcpy(&(new_tail->first_block), (buf + 36), sizeof(uint8_t) * 2);
    memcpy(&(new_tail->type), (buf + 38), sizeof(uint8_t) * 1);
    memcpy(&(new_tail->perm), (buf + 39), sizeof(uint8_t) * 1);
    memcpy(&(new_tail->mtime), (buf + 40), sizeof(uint8_t) * 8);
    tail->next = new_tail;
    return new_tail;
}

void print_perm(uint8_t perm) {
  char output[4];
  if (perm == 0) {
    strcpy(output, "---");
  } else if (perm == 2) {
    strcpy(output, "-w-");
  } else if (perm == 4) {
    strcpy(output, "r--");
  } else if (perm == 5) {
    strcpy(output, "r-x");
  } else if (perm == 6) {
    strcpy(output, "rw-");
  } else if (perm == 7) {
    strcpy(output, "rwx");
  }
  printf("%s ", output);
}

void print_date_time(time_t mtime) {
  struct tm *tm = localtime(&mtime);
  int month = tm->tm_mon + 1;  
  char month_str[4];
  if (month == 1) {
    strcpy(month_str, "Jan");
  } else if (month == 2) {
    strcpy(month_str, "Feb");
  } else if (month == 3) {
    strcpy(month_str, "Mar");
  } else if (month == 4) {
    strcpy(month_str, "Apr");
  } else if (month == 5) {
    strcpy(month_str, "May");
  } else if (month == 6) {
    strcpy(month_str, "Jun");
  } else if (month == 7) {
    strcpy(month_str, "Jul");
  } else if (month == 8) {
    strcpy(month_str, "Aug");
  } else if (month == 9) {
    strcpy(month_str, "Sep");
  } else if (month == 10) {
    strcpy(month_str, "Oct");
  } else if (month == 11) {
    strcpy(month_str, "Nov");
  } else if (month == 12) {
    strcpy(month_str, "Dec");
  }
  int day = tm->tm_mday;
  int hour = tm->tm_hour;
  int minute = tm->tm_min;
  int second = tm->tm_sec;
  printf("%s %d %d:%d:%d ", month_str, day, hour, minute, second);
}

void print_fs_file_info(fs_file* file) {
  printf("%u ", file->first_block);
  print_perm(file->perm);
  printf("%u ", file->size);
  print_date_time(file->mtime);
  printf("%s\n", file->name);
}