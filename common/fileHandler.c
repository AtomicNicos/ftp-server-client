#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>


#include "fileHandler.h"

int isValidPath(char *path) {
    struct stat* statBuffer = malloc(sizeof(struct stat));
    int status = lstat(path, statBuffer);
    __mode_t mode = statBuffer->st_mode;
    
    free(statBuffer);

    return (status == 0 && (S_ISDIR(mode) || S_ISLNK(mode) || S_ISREG(mode))) ? 1 : 0;
}

long long getLength(char *path) {
    if (isValidPath(path) == 1) {
        long long length;
        FILE *f = fopen(path, "rb");

        if (f) {
            fseek(f, 0, SEEK_END);
            length = ftell(f);
            fseek(f, 0, SEEK_SET);
            
            return length;
        } else
            return -1;
    } else 
        return -1;
}