#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <fcntl.h>

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

    unsigned char *instruction = malloc(INSTR_SIZE + 1); unsigned char *data = malloc(BUFFER_SIZE + 1);
    C_ALL(instruction, data);
    snprintf(instruction, INSTR_SIZE + 1, "%s", CMD_LIST);
    sendData(localSocket, instruction, data, 0);   // 00 OUT : LIST
    DEBUG("=> LIST\n");
    usleep(1);
    recvData(localSocket, instruction, data);   // 01 IN  : FILES <num>
    ODEBUG("<= %s\n", instruction);

    long fCount = strtol(instruction + CMD_LEN + 1, NULL, 0);
    printf("%s %lu\n", instruction, fCount);

    printf("[%ld] Remote Files\n", fCount);
    for (long i = 0; i < fCount; i++) {
        C_ALL(instruction, data);
        recvData(localSocket, instruction, data);   // 02 IN  : FILE <name>
        sendData(localSocket, STATUS_OK, "", 0);    // 03 OUT : OK
        printf("  [%ld] => %s\n", i + 1, data);
    }
    
    free(instruction); free(data);
    return "builtin list";
}

char* upload(int localSocket, int *_argc, char *argv0, char **_argv) {
    if (*_argc != 2 && *_argc != 3)
        (WARN("-upload: invalid amount of arguments.", "ul <file> [<file new name>]"));

    int renameMode = (*_argc == 3) ? 1 : 0;
    unsigned char *instruction = malloc(INSTR_SIZE + 1); unsigned char *data = malloc(BUFFER_SIZE + 1);
    char *localFilePath = malloc(FILENAME_MAX + 1);
    C_ALL(instruction, data); memset(localFilePath, 0, FILENAME_MAX + 1);

    snprintf(localFilePath, FILENAME_MAX, "%s/%s", getFilesFolder(argv0), _argv[1]);
    ull fileSize = getLength(localFilePath);

    if (fileSize > 0)
        printf("FILE SIZE IS %lld BYTES\n", fileSize);
    else 
        (WARN("-upload: specified file does not exist.", "ul <file> [<file new name>]"));

    snprintf(instruction, INSTR_SIZE + 1, "%s 0x%.17llx", CMD_UPLOAD, fileSize);
    sendData(localSocket, instruction, data, 0);    // 00 OUT : UPLOAD
    C_ALL(instruction, data);
    recvData(localSocket, instruction, data);       // 00.1 IN : OK

    C_ALL(instruction, data);
    snprintf(instruction, INSTR_SIZE + 1, "%s", CMD_NAME);
    snprintf(data, BUFFER_SIZE + 1, "%s", (renameMode == 1) ? _argv[2] : _argv[1]);
    sendData(localSocket, instruction, data, strlen((renameMode == 1) ? _argv[2] : _argv[1])); // 01 OUT : NAME <value>
    DEBUG("=> NAME\n");

    C_ALL(instruction, data);
    recvData(localSocket, instruction, data);       // 02|05|06 IN  : [OVERWRITE|ERROR|OK]
    ODEBUG("<= %s\n", instruction);

    if (strncmp(instruction, CMD_OVERWRITE, CMD_LEN) == 0) {
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
        
        CINST(instruction);
        if (change == 0) {
            printf("Client chose to ABORT.\n");
            snprintf(instruction, INSTR_SIZE + 1, "%s", STATUS_ABORT);
            sendData(localSocket, instruction, data, 0);    // 03 OUT : ABORT
            DEBUG("=> ABORT\n");
            free(localFilePath); free(instruction); free(data);
            return "NO OVERWRITE EXIT";
        } else {
            snprintf(instruction, INSTR_SIZE + 1, "%s", STATUS_OK);
            sendData(localSocket, instruction, data, 0);      // 04 OUT : OK
            DEBUG("=> OK\n");
        }

        C_ALL(instruction, data);
        recvData(localSocket, instruction, data);   // 05|06|10 IN  : [ERROR|OK|RESSOURCE_IN_USE]
        ODEBUG("<= %s\n", instruction);
    }
    
    // Server notifies that the file is locked.
    if (strncmp(instruction, STATUS_RESINUSE, CMD_LEN) == 0) { 
        printf("The resource is currently unavailable (java.io.ConcurrentModificationException)\n");
        free (localFilePath); free(instruction); free(data);
        return "RIU";
    } else if (strncmp(instruction, STATUS_ERR, CMD_LEN) == 0) { // Server notifies that the file is locked.
        printf("Could not create the file server side.\n");
        free (localFilePath); free(instruction); free(data);
        return "ERR";
    } else if (strncmp(instruction, STATUS_OK, CMD_LEN) == 0) {
        int fd = open(localFilePath, O_RDONLY, 0600);
        
        if (fd == -1) {
            perror("FOPEN");
            snprintf(instruction, INSTR_SIZE + 1, "%s", STATUS_ERR);
            sendData(localSocket, instruction, data, 0); // 07 OUT : ERROR
            DEBUG("=> ERROR\n");
        } else {
            snprintf(instruction, INSTR_SIZE + 1, "%s", STATUS_OK);
            sendData(localSocket, instruction, data, 0);  // 08 OUT : OK
            DEBUG("=> OK\n");

            recvData(localSocket, instruction, data);   // 08.1 IN  : OK
            ODEBUG("<= %s\n", instruction);
            
            pushFile(localSocket, fd);

            C_ALL(instruction, data);
            recvData(localSocket, instruction, data);   // 09 IN  : DONE
            ODEBUG("<= %s\n", instruction);
        }
        close(fd);
    }

    C_ALL(instruction, data);
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