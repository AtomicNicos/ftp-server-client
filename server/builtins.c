#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "builtins.h"
#include "../common/utils.h"
#include "../common/fileHandler.h"

void queryList(int localSocket, char *argv0) {
    char *instruction = malloc(INSTR_SIZE + 1); char *data = malloc(BUFFER_SIZE + 1);
    char *dirPath = malloc(FILENAME_MAX + 1); char *execPath = malloc(FILENAME_MAX + 1);
    char *files[FILENAME_MAX];
    int *fileCount = malloc(sizeof(int));

    snprintf(execPath, FILENAME_MAX + 1, "%s", argv0 + 1);

    char *slash = strrchr(execPath, '/');
    if (slash)
        slash[0] = '\0';
    
    snprintf(dirPath, FILENAME_MAX + 1, "%s%s/~", getenv("PWD"), execPath);
    getFiles(dirPath, files, fileCount);

    memset(instruction, 0, INSTR_SIZE + 1); memset(data, 0, BUFFER_SIZE + 1);
    snprintf(instruction, INSTR_SIZE + 1, "FILES 0x%.4x", *fileCount);

    usleep(1);
    sendData(localSocket, instruction, data);

    for (int i = 0; i < *fileCount; i++) {
        memset(instruction, 0, INSTR_SIZE + 1); memset(data, 0, BUFFER_SIZE + 1);   
        usleep(1);
        
        snprintf(instruction, INSTR_SIZE + 1, "FILE 0x%.4x", i + 1);
        snprintf(data, BUFFER_SIZE + 1, "%s", files[i]);
        sendData(localSocket, instruction, data);
    }
    
    for (int i = 0; i < *fileCount; i++)
        files[i] = realloc(files[i], 0);

    free(execPath); free(dirPath);
    free(fileCount);
    free(instruction); free(data);
}


void getFile(int localSocket, char *argv0, char init[INSTR_SIZE]) {
    printf("%s\n", init);
    ull size = strtoull(init + 3, NULL, 0);
    ull currentSize = (ull) 0;
    //printf("SIZE %.17llx = %llu\n", size, size);

    char *instruction = malloc(INSTR_SIZE + 1); char *data = malloc(BUFFER_SIZE + 1);
    int len = recvData(localSocket, instruction, data);
    printf("NAME => [%s]\n", data);

    /* 
    <= fname [NAME][new NAME]
    check if exists.
    if exists:
        => [OVERWRITE][]
        <= [CONFIRM|DENY][]
        if CONFIRM:
            => [OK][]
        else:
            => [CANCEL][]

    => [OK][]
    while (size):
        <= [PACKET][BYTES]
        write BYTES
        
    */
}