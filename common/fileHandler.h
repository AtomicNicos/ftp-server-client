#ifndef HEADER_COMMON_FILES
#define HEADER_COMMON_FILES

#include "utils.h"

int isValidPath(char *path);

sll getLength(char *path);

void getFiles(const char *path, char *files[FILENAME_MAX + 1], int *numberOfFiles);

#endif