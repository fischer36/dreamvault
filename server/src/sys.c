#include "sys.h"

#include <sys/stat.h>

#ifdef __linux__
#include <dirent.h>
#else
#include <windows.h>
#endif

bool sys_folder_exists(const char *folderPath) {
  bool exists = false;
#ifdef __linux__
  DIR *dir = opendir(folderPath);
  if (dir != NULL) {
    closedir(dir);
    exists = true;
  } else {

    exists = false;
  }
#else
  DWORD attributes = GetFileAttributes(folderPath);

  if (attributes != INVALID_FILE_ATTRIBUTES &&
      (attributes & FILE_ATTRIBUTE_DIRECTORY)) {

    printf("Folder exists\n");
    exists = true;
  } else {
    printf("Folder does not exist\n");
    exists = false;
  }

#endif

  return exists;
}

int sys_folder_create(const char *folderPath) {
#ifdef __linux__

  if (mkdir(folderPath, 0700) == 0) {
    return 0;
  }

#else
  if (CreateDirectory(folderPath, NULL) == 0) {
    return 0;
  }
#endif

  return 1;
}

int sys_file_write(const char *filePath, const char *data) {
  FILE *file = fopen(filePath, "wb");
  if (file == NULL) {
    printf("[error] failed to open file for writing\n");
    return -1;
  }

  size_t dataSize = strlen(data);

  if (fwrite(data, 1, dataSize, file) < dataSize) {
    printf("[error] failed writing to file\n");
    fclose(file);
    return -2;
  }

  fclose(file);

  return 0;
}

bool sys_file_exists(const char *filePath) {

  FILE *file = fopen(filePath, "r");
  if (file == NULL) {
    return false;
  }
  fclose(file);
  return true;
}

int sys_file_create(const char *filePath) {

  FILE *file = fopen(filePath, "w");
  if (file == NULL) {
    return -1;
  }

  fclose(file);
  return 0;
}

int sys_file_delete(const char *filePath) {

  if (remove(filePath) != 0) {
    return -1;
  } else {
    return 0;
  }
}

int sys_file_rename(const char *oldPath, const char *newPath) {

  printf("Renaming %s to %s\n", oldPath, newPath);
  if (sys_file_exists(oldPath) == false) {
    fprintf(stderr, "File does not exist\n %s\n", oldPath);
    return -1;
  }

  if (sys_file_exists(newPath) == true) {
    fprintf(stderr, "File already exists\n %s\n", newPath);
    return -1;
  }
  if (rename(oldPath, newPath) != 0) {
    perror("Error renaming file");
    return -1;
  }
  printf("File renamed successfully: %s to %s\n", oldPath, newPath);
  return 0;
}

int sys_file_read(const char *file_path, char **buffer, size_t *size) {
  FILE *file = fopen(file_path, "rb");
  if (!file) {
    perror("Failed to open file");
    return -1;
  }

  fseek(file, 0, SEEK_END);
  long file_size = ftell(file);
  rewind(file);

  *buffer = (char *)malloc(file_size + 1);
  if (!*buffer) {
    fclose(file);
    perror("Failed to allocate memory");
    return -1;
  }

  size_t bytes_read = fread(*buffer, 1, file_size, file);
  if (bytes_read < (size_t)file_size) {
    free(*buffer);
    fclose(file);
    perror("Failed to read the entire file");
    return -1;
  }

  (*buffer)[file_size] = '\0';

  fclose(file);

  *size = (size_t)file_size;
  return 0;
}

int sys_get_file_size(const char *filePath, long *size) {
  FILE *file = fopen(filePath, "rb");
  if (file == NULL) {
    perror("Unable to open file");
    return -1;
  }

  if (fseek(file, 0, SEEK_END) != 0) {
    fclose(file);
    perror("Seeking in file failed");
    return -1;
  }

  long fileSize = ftell(file);
  if (fileSize == -1) {
    fclose(file);
    perror("Getting file size failed");
    return -1;
  }

  fclose(file);

  *size = fileSize;

  return 0;
}
