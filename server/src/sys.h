
#pragma once

#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#define MAX_FILE_SIZE 256

// TODO
int sys_file_write_index(const char *filePath, const char *data, long index);

bool sys_folder_exists(const char *folderPath);
int sys_folder_create(const char *folderPath);
int sys_get_file_size(const char *filePath, long *size);
bool sys_file_exists(const char *filePath);
int sys_file_create(const char *filePath);
int sys_file_delete(const char *filePath);
int sys_file_rename(const char *oldPath, const char *newPath);
int sys_file_write(const char *filePath, const char *data, size_t *size);
int sys_file_read(const char *file_path, char **buffer,
                  size_t *size); /* Must free buffer */
