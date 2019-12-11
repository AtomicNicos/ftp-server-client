#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <fcntl.h>


#include "builtins.h"
#include "../common/utils.h"
#include "../common/fileHandler.h"

void queryList(int localSocket, char *argv0) {
    char *instruction = malloc(INSTR_SIZE + 1); char *data = malloc(BUFFER_SIZE + 1);
    char *files[FILENAME_MAX];
    int *fileCount = malloc(sizeof(int));

    getFiles(getFilesFolder(argv0), files, fileCount);

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

    free(fileCount);
    free(instruction); free(data);
}


void getFile(int localSocket, char *argv0, char init[INSTR_SIZE]) {
    printf("%s\n", init);
    ull size = strtoull(init + 3, NULL, 0);
    ull currentSize = (ull) 0;
    ull fileChunks = size / 4098L + 1L;

    char *instruction = malloc(INSTR_SIZE + 1); char *data = malloc(BUFFER_SIZE + 1);
    int len = recvData(localSocket, instruction, data);
    printf("NAME => [%s]\n", data);

    char *localFilePath = malloc(FILENAME_MAX + 1);
    snprintf(localFilePath, FILENAME_MAX + 1, "%s/%s", getFilesFolder(argv0), data);
    
    memset(instruction, 0, INSTR_SIZE + 1); memset(data, 0, BUFFER_SIZE + 1);

    if (isValidPath(localFilePath) == 1) {
        printf("EXISTS SO OVERRIDE ?\n");
        snprintf(instruction, INSTR_SIZE + 1, "%s", CMD_OVERRIDE);
        usleep(1);
        sendData(localSocket, instruction, data);
        usleep(1);

        memset(instruction, 0, INSTR_SIZE + 1); memset(data, 0, BUFFER_SIZE + 1);
        recvData(localSocket, instruction, data);

        if (strncmp(instruction, STATUS_ERR, strlen(STATUS_ERR)) == 0) {        
            free(localFilePath);
            return;
        }
    }

    // TODO ? Backup system ?
    if (isLocked(localFilePath) == 0) {
        lockFile(localFilePath);
        FILE *fd = fopen(localFilePath, "wb+");

        if (fd == NULL) {
            perror("FOPEN");
            snprintf(instruction, INSTR_SIZE + 1, STATUS_ERR);
            sendData(localSocket, instruction, data);
        } else {
            snprintf(instruction, INSTR_SIZE + 1, STATUS_OK);
            sendData(localSocket, instruction, data);

            memset(instruction, 0, INSTR_SIZE + 1); memset(data, 0, BUFFER_SIZE + 1);
            recvData(localSocket, instruction, data);

            if (strncmp(instruction, STATUS_ERR, strlen(STATUS_ERR)) == 0) {
                printf("CLIENT COULD NOT OPEN FILE, ABORT\n");
            } else { // OK.
                memset(instruction, 0, INSTR_SIZE + 1); memset(data, 0, BUFFER_SIZE + 1);
                printf("RECEIVING %lld bytes => %lld packets of data\n", size, (size / BUFFER_SIZE) + 1);
                fprintf(fd, "%s", "lock");
                sleep(10);
            }
        }
        unlockFile(localFilePath);
        // ! WRITE TO FILE AT PATH
        fclose(fd);
    } else {
        snprintf(instruction, INSTR_SIZE + 1, STATUS_RESINUSE);
        sendData(localSocket, instruction, data);
    }
    isLocked(localFilePath);
    
    free(localFilePath);

    /* 

    while (size):
        <= [PACKET][BYTES]
        write BYTES
    => [DONE][]
    */
}