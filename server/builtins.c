#define _GNU_SOURCE

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

    C_ALL(instruction, data);
    snprintf(instruction, INSTR_SIZE + 1, "%s 0x%.4x", CMD_FILES, *fileCount);

    sendData(localSocket, instruction, data, 0);    // 01 OUT : FILES <num>

    for (int i = 0; i < *fileCount; i++) {
        usleep(5);
        C_ALL(instruction, data); 
        snprintf(instruction, INSTR_SIZE + 1, "%s 0x%.4x", CMD_FILE, i + 1);
        snprintf(data, BUFFER_SIZE + 1, "%s", files[i]);
        sendData(localSocket, instruction, data, strlen(files[i])); // 02 OUT : FILE <name>

        C_ALL(instruction, data);
        recvData(localSocket, instruction, data);       // 03 IN  : OK
    }
    
    for (int i = 0; i < *fileCount; i++)
        files[i] = realloc(files[i], 0);

    free(fileCount);
    free(instruction); free(data);
}

/**
 * TODO
 */
void receiveUpload(int localSocket, char *argv0, unsigned char init[INSTR_SIZE]) {
    ull fileSize = strtoull(init + CMD_LEN + 1, NULL, 0);
    printf("FILE SIZE %llu\n", fileSize);

    unsigned char *instruction = malloc(INSTR_SIZE + 1); unsigned char *data = malloc(BUFFER_SIZE + 1);
    char *localFilePath = malloc(FILENAME_MAX + 1);
    C_ALL(instruction, data); memset(localFilePath, 0, FILENAME_MAX + 1);

    usleep(2);
    snprintf(instruction, INSTR_SIZE + 1, "%s", STATUS_OK);
    sendData(localSocket, instruction, data, 0);  // 00.1 OUT : OK
    DEBUG("=> OK\n");

    int len = recvData(localSocket, instruction, data); // 01 IN  : NAME <value>
    ODEBUG("<= %s\n", instruction);

    snprintf(localFilePath, FILENAME_MAX + 1, "%s/%s", getFilesFolder(argv0), data);

    // Determine if file should be overwritten, ie. if it exists locally.
    if (isValidPath(localFilePath) == 1) {
        snprintf(instruction, INSTR_SIZE + 1, "%s", CMD_OVERWRITE);
        sendData(localSocket, instruction, data, 0);   // 02 OUT : OVERWRITE
        DEBUG("=> OVERWRITE\n");

        C_ALL(instruction, data);
        recvData(localSocket, instruction, data);       // 03|04 IN  : [ABORT|OK]
        ODEBUG("<= %s\n", instruction);

        if (strncmp(instruction, STATUS_ABORT, CMD_LEN) == 0) {        
            free(localFilePath); free(instruction); free(data);
            return;
        } else
            remove(localFilePath);
    }
    
    // If the file is not being used by another client (upload or download), because data corruption.
    if (isLocked(localFilePath) == 0) {
        lockFile(localFilePath);
        int fd = open(localFilePath, O_CREAT | O_RDWR, 0600);

        if (fd == -1) { // If the file could not be created or opened.
            perror("FOPEN");
            snprintf(instruction, INSTR_SIZE + 1, "%s", STATUS_ERR);
            sendData(localSocket, instruction, data, 0);     // 05 OUT : ERROR
            DEBUG("=> ERROR\n");
        } else {
            snprintf(instruction, INSTR_SIZE + 1, "%s", STATUS_OK);
            sendData(localSocket, instruction, data, 0);      // 06 OUT : OK
            DEBUG("=> OK\n");
            C_ALL(instruction, data);

            recvData(localSocket, instruction, data);       // 07|08 IN  : [ERROR|OK]
            ODEBUG("<= %s\n", instruction);
            if (strncmp(instruction, STATUS_ERR, CMD_LEN) == 0)
                printf("CLIENT COULD NOT OPEN FILE, ABORT\n");
            else {
                snprintf(instruction, INSTR_SIZE + 1, "%s", STATUS_OK);
                sendData(localSocket, instruction, data, 0);  // 08.1 OUT : OK
                DEBUG("=> OK\n");
                pullFile(localSocket, fd, fileSize);
            }
        }

        C_ALL(instruction, data);
        snprintf(instruction, INSTR_SIZE + 1, "%s", STATUS_DONE);
        sendData(localSocket, instruction, data, 0);        // 09 OUT : DONE
        DEBUG("=> DONE\n");

        close(fd);
        unlockFile(localFilePath);
        printf("Transfer of %s finished.\n", localFilePath);
    } else {
        snprintf(instruction, INSTR_SIZE + 1, "%s", STATUS_RESINUSE);
        sendData(localSocket, instruction, data, 0);        // 10 OUT : RESSOURCE_IN_USE
        DEBUG("=> RIU\n");
    }
    
    free(instruction); free(data);
    free(localFilePath);
}