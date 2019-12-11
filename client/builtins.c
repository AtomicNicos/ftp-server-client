#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "../common/utils.h"
#include "../common/fileHandler.h"
#include "builtins.h"
#include "lineReader.h"

/** @brief Prints a nice prompt.
 * @param user The user name
 * @param host The Server IP
 * @param cwd The current working directory
 */
void print_prompt(char *user, char *host, char *cwd) {
    printColorized(user, 32, 40, 0, 0);
    printf("@");
    printColorized(host, 35, 40, 0, 0);
    printf(":");
    printColorized(cwd, 36, 40, 0, 0);
    printf("$ ");
}

/** @brief Queries a list of files from the server.
 * @note Max 0xFFFF files.
 * @param all See @link { executeBuiltin } */
char* list(int localSocket, int *_argc, char *argv0, char **_argv) {
    if (*_argc != 1)
        (WARN("-list: too many arguments.", "list"));

    char *instruction = malloc(INSTR_SIZE + 1); char *data = malloc(BUFFER_SIZE + 1);
    memset(instruction, 0, INSTR_SIZE + 1); memset(data, 0, BUFFER_SIZE + 1);
    sendData(localSocket, CMD_LIST, data);
    
    usleep(1);
    recvData(localSocket, instruction, data);

    long fCount = strtol(instruction + 6, NULL, 0);
    printf("[%ld] Remote Files\n", fCount);
    for (long i = 0; i < fCount; i++) {
        memset(instruction, 0, INSTR_SIZE + 1); memset(data, 0, BUFFER_SIZE + 1);
        usleep(1);
        recvData(localSocket, instruction, data);
        printf("  [%ld] => %s\n", i + 1, data);
    }
    
    free(instruction); free(data);
    return "builtin list";
}

char* upload(int localSocket, int *_argc, char *argv0, char **_argv) {
    if (*_argc != 2 && *_argc != 3)
        (WARN("-upload: invalid amount of arguments.", "ul <file> [<file new name>]"));

    int renameMode = (*_argc == 3) ? 1 : 0;

    char *localFilePath = malloc(FILENAME_MAX + 1);
    snprintf(localFilePath, FILENAME_MAX, "%s/%s", getFilesFolder(argv0), _argv[1]);
    ull fileLength = getLength(localFilePath);
    ull fileChunks = fileLength / 4098L + 1L;

    if (fileLength > 0)
        printf("FILE LEN %lld\n", fileLength);
    else 
        (WARN("-upload: specified file does not exist.", "ul <file> [<file new name>]"));

    char *instruction = malloc(INSTR_SIZE + 1); char *data = malloc(BUFFER_SIZE + 1);
    memset(instruction, 0, INSTR_SIZE + 1); memset(data, 0, BUFFER_SIZE + 1);

    snprintf(instruction, INSTR_SIZE + 1, "%s 0x%.17llx", CMD_UPLOAD, fileLength);
    sendData(localSocket, instruction, data);
    
    memset(instruction, 0, INSTR_SIZE + 1);
    snprintf(instruction, INSTR_SIZE + 1, "%s", CMD_NAME);
    snprintf(data, BUFFER_SIZE + 1, "%s", (renameMode == 1) ? _argv[2] : _argv[1]);
    
    usleep(1);
    sendData(localSocket, instruction, data);
    
    memset(instruction, 0, INSTR_SIZE + 1); memset(data, 0, BUFFER_SIZE + 1);
    usleep(1);
    recvData(localSocket, instruction, data);

    if (strncmp(instruction, CMD_OVERRIDE, strlen(CMD_OVERRIDE)) == 0) {
        printf("`%s` already exists on the server. OVERWRITE ? [Y/n] > ", (renameMode == 1) ? _argv[2] : _argv[1]);
        char *result;
        int change = -1;
        while (change == -1) {
            result = getLine();
            if (strlen(result) == 1 && (strncmp(result, "Y", 1) == 0 || strncmp(result, "y", 1) == 0))
                change = 1;
            else if (strlen(result) == 1 && (strncmp(result, "N", 1) == 0 || strncmp(result, "n", 1) == 0))
                change = 0;
            else
                printf("Invalid answer. OVERWRITE ? [Y/n] > ");
            free(result);
        } 
        
        usleep(1);
        memset(instruction, 0, INSTR_SIZE + 1);
        if (change == 0) {
            printf("Client chose to ABORT.\n");
            snprintf(instruction, INSTR_SIZE + 1, "%s", STATUS_ERR);
            sendData(localSocket, instruction, data);
            free(localFilePath);
            return "NO OVERWRITE EXIT";
        } else {
            snprintf(instruction, INSTR_SIZE + 1, "%s", STATUS_OK);
            sendData(localSocket, instruction, data);
        }
        memset(instruction, 0, INSTR_SIZE + 1); memset(data, 0, BUFFER_SIZE + 1);
        usleep(1);
        recvData(localSocket, instruction, data);
    } else {
        printf("NO OVERWRITE\n");
    }

    if (strncmp(instruction, STATUS_RESINUSE, strlen(STATUS_RESINUSE)) == 0) { // Server notifies that the file is locked.
        printf("The resource is currently unavailable (java.io.ConcurrentModificationException)\n");
    } else if (strncmp(instruction, STATUS_ERR, strlen(STATUS_ERR)) == 0) { // Server notifies that the file is locked.
        printf("Could not create the file server side.\n");
    } else if (strncmp(instruction, STATUS_OK, strlen(STATUS_OK)) == 0) {
        printf("UPLOAD %lld bytes start\n", fileLength);
        FILE *fd = fopen(localFilePath, "rb");
        
        if (fd == NULL) {
            perror("FOPEN");
            snprintf(instruction, INSTR_SIZE + 1, "%s", STATUS_ERR);
            sendData(localSocket, instruction, data);
        } else {
            snprintf(instruction, INSTR_SIZE + 1, "%s", STATUS_OK);
            sendData(localSocket, instruction, data);

            while (n_read = read(fd_src, buffer, sizeof buffer), n_read > 0) {  // While bytes can be read from the src file.
            }
        }
    }
    
    memset(instruction, 0, INSTR_SIZE + 1); memset(data, 0, BUFFER_SIZE + 1);
    printf("HERE\n");
    free(localFilePath); free(instruction); free(data);
    
    return "builtin upload";
}

char* download(int localSocket, int *_argc, char *argv0, char **_argv) {
    if (*_argc < 2)
        WARN("-download: insufficient arguments.", "dl <file> [...]");
    return "builtin download";
}

char* getHelp(int localSocket, int *_argc, char *argv0, char **_argv) {
    printf("Built-ins :\n- help\n- cd [<dir>]\n- list\n- ul <file> [...]\n- dl <file> [...]\n");
    return "NO";
}

/** Define the functions */
char* (*builtin[]) (int, int *, char*, char **) = {
    &list,
    &upload,
    &download,
    &getHelp
};

/** Define the builtins */
char *builtins[] = {
    "list",
    "ul",
    "dl",
    "help"
};

/** @brief How intuitively well-named can a function be ?
 * @param _argv The arguments vector.
 */
char* executeBuiltin(int localSocket, int *_argc, char *argv0, char **_argv) {
    for (int i = 0; i < BUILTINS_COUNT; i++)
        if (strcmp(_argv[0], builtins[i]) == 0) 
            return (*builtin[i])(localSocket, _argc, argv0, _argv);
    return NULL;
}