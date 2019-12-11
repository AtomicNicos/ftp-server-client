#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <fcntl.h>
#include <errno.h>


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
    sendData(localSocket, instruction, data, 0);

    for (int i = 0; i < *fileCount; i++) {
        memset(instruction, 0, INSTR_SIZE + 1); memset(data, 0, BUFFER_SIZE + 1);   
        snprintf(instruction, INSTR_SIZE + 1, "FILE 0x%.4x", i + 1);
        snprintf(data, BUFFER_SIZE + 1, "%s", files[i]);
        usleep(4);
        sendData(localSocket, instruction, data, strlen(files[i]));
        recvData(localSocket, instruction, data);
    }
    
    for (int i = 0; i < *fileCount; i++)
        files[i] = realloc(files[i], 0);

    free(fileCount);
    free(instruction); free(data);
}


void getFile(int localSocket, char *argv0, unsigned char init[INSTR_SIZE]) {
    ull size = strtoull(init + 3, NULL, 0);

    unsigned char *instruction = malloc(INSTR_SIZE + 1); unsigned char *data = malloc(BUFFER_SIZE + 1);
    char *localFilePath = malloc(FILENAME_MAX + 1);

    int len = recvData(localSocket, instruction, data);
    snprintf(localFilePath, FILENAME_MAX + 1, "%s/%s", getFilesFolder(argv0), data);
    printf("1 RCVD <%s>\n", instruction);
    sleep(1);
    
    sendData(localSocket, STATUS_OK, "", 0);
    printf("2 SENT <%s>\n", STATUS_OK);
    memset(instruction, 0, INSTR_SIZE + 1); memset(data, 0, BUFFER_SIZE + 1);

    sleep(1);
    // Determine if file should be overwritten.
    if (isValidPath(localFilePath) == 1) {
        printf("EXISTS SO OVERRIDE ?\n");
        snprintf(instruction, INSTR_SIZE + 1, "%s", CMD_OVERRIDE);
        sendData(localSocket, instruction, data, 0);
        printf("3 SENT <%s>\n", instruction);
        sleep(1);

        memset(instruction, 0, INSTR_SIZE + 1); memset(data, 0, BUFFER_SIZE + 1);
        recvData(localSocket, instruction, data);
        
        printf("4 RCVD <%s>\n", instruction);
        sleep(1);

        if (strncmp(instruction, STATUS_ERR, strlen(STATUS_ERR)) == 0) {        
            free(localFilePath);
            return;
        } else {
            remove(localFilePath);
        }
    }

    // TODO ? Backup system ?
    if (isLocked(localFilePath) == 0) {
        lockFile(localFilePath);
        int fd = open(localFilePath, O_CREAT | O_RDWR, 0600);

        if (fd == -1) {
            perror("FOPEN");
            snprintf(instruction, INSTR_SIZE + 1, STATUS_ERR);
            sendData(localSocket, instruction, data, 0);
            printf("5 SENT <%s>\n", instruction);
        } else {
            snprintf(instruction, INSTR_SIZE + 1, STATUS_OK);
            sendData(localSocket, instruction, data, 0);
            printf("5 SENT <%s>\n", instruction);

            sleep(1);
            memset(instruction, 0, INSTR_SIZE + 1); memset(data, 0, BUFFER_SIZE + 1);
            recvData(localSocket, instruction, data);
            printf("6 RCVD <%s>\n", instruction);

            if (strncmp(instruction, STATUS_ERR, strlen(STATUS_ERR)) == 0) {
                printf("CLIENT COULD NOT OPEN FILE, ABORT\n");
            } else { // OK.
                ull n_total = 0;
                while (n_total < size) {
                    memset(instruction, 0, INSTR_SIZE + 1); memset(data, 0, BUFFER_SIZE + 1);
                    int n_read = recvData(localSocket, instruction, data);
                    printf("7 RCVD <%s>\n", instruction);
                    printf("DATA <%s>\n\n", data);
                    n_total += n_read;
                    //printf("READ %d / %lld\n", n_read, n_total);
                    //usleep(1);
                    if (strncmp(instruction, STATUS_DONE, strlen(STATUS_DONE) != 0)) {
                        char *buffer_ptr = data;
                        ull nwritten;
                        if (n_total > size) {
                            n_read -= (n_total - size);
                        }
                        do {
                            nwritten = write(fd, data, n_read);
                            if (nwritten >= 0) {
                                n_read -= nwritten;
                                buffer_ptr += nwritten;
                            } else if (errno != EINTR) {
                                return;
                            }
                        } while (n_read > 0);
                    }
                }
            }
            
            sendData(localSocket, STATUS_OK, "", 0);
            printf("8 SENT <%s>\n", instruction);
        }

        sleep(1);
        memset(instruction, 0, INSTR_SIZE + 1); memset(data, 0, BUFFER_SIZE + 1);
        recvData(localSocket, instruction, data);
        printf("9 RCVD <%s>\n", instruction);
        unlockFile(localFilePath);
        // ! WRITE TO FILE AT PATH
        close(fd);
    } else {
        snprintf(instruction, INSTR_SIZE + 1, STATUS_RESINUSE);
        sendData(localSocket, instruction, data, 0);
        printf("10 SENT <%s>\n", instruction);
    }
    
    free(localFilePath);
}