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
    DEBUG("=> LIST");
    
    C_ALL(instruction, data);
    recvData(localSocket, instruction, data);   // 01 IN  : FILES <num>
    ODEBUG("<= %s", instruction);

    long fCount = strtol(instruction + CMD_LEN + 1, NULL, 0);

    C_ALL(instruction, data);
    snprintf(instruction, INSTR_SIZE + 1, "%s", STATUS_OK);
    sendData(localSocket, instruction, data, 0);   // 02 OUT : OK
    DEBUG("=> OK");

    printf("[%ld] Remote Files\n", fCount);
    for (long i = 0; i < fCount; i++) {
        C_ALL(instruction, data);
        recvData(localSocket, instruction, data);   // 03 IN  : FILE <name>
        ODEBUG("<= %s", instruction);
        printf("  [%ld] => %s\n", i + 1, data);

        C_ALL(instruction, data);
        snprintf(instruction, INSTR_SIZE + 1, "%s", STATUS_OK);
        sendData(localSocket, instruction, data, 0);    // 04 OUT : OK
        DEBUG("=> OK");
    }

    C_ALL(instruction, data);
    free(instruction); free(data);
    return "builtin list";
}

/**
 * TODO
 */
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
        printf("Transfer of %s (%lld bytes)\n", _argv[1], fileSize);
    else 
        (WARN("-upload: specified file does not exist.", "ul <file> [<file new name>]"));

    snprintf(instruction, INSTR_SIZE + 1, "%s 0x%.17llx", CMD_UPLOAD, fileSize);
    sendData(localSocket, instruction, data, 0);    // 00 OUT : UPLOAD
    DEBUG("=> UPLOAD");

    C_ALL(instruction, data);
    recvData(localSocket, instruction, data);       // 00.1 IN : OK
    ODEBUG("<= %s", instruction);

    C_ALL(instruction, data);
    snprintf(instruction, INSTR_SIZE + 1, "%s", CMD_NAME);
    snprintf(data, BUFFER_SIZE + 1, "%s", (renameMode == 1) ? _argv[2] : _argv[1]);
    sendData(localSocket, instruction, data, strlen((renameMode == 1) ? _argv[2] : _argv[1])); // 01 OUT : NAME <value>
    DEBUG("=> NAME");

    C_ALL(instruction, data);
    recvData(localSocket, instruction, data);       // 02|05|06 IN  : [OVERWRITE|ERROR|OK]
    ODEBUG("<= %s", instruction);

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
            DEBUG("=> ABORT");
            
            C_ALL(instruction, data);
            free(localFilePath); free(instruction); free(data);
            return "NO OVERWRITE EXIT";
        } else {
            snprintf(instruction, INSTR_SIZE + 1, "%s", STATUS_OK);
            sendData(localSocket, instruction, data, 0);      // 04 OUT : OK
            DEBUG("=> OK");
        }

        C_ALL(instruction, data);
        recvData(localSocket, instruction, data);   // 05|06|10 IN  : [ERROR|OK|RESSOURCE_IN_USE]
        ODEBUG("<= %s", instruction);
    }
    
    // Server notifies that the file is locked.
    if (strncmp(instruction, STATUS_RESINUSE, CMD_LEN) == 0) { 
        printColorized("The file is currently unavailable (java.io.ConcurrentModificationException)", 32, 40, 0, 1);
        free (localFilePath); free(instruction); free(data);
        return "RIU";
    } else if (strncmp(instruction, STATUS_ERR, CMD_LEN) == 0) { // Server notifies that the file is locked.
        printColorized("Could not create the file server side.", 32, 40, 0, 1);
        free (localFilePath); free(instruction); free(data);
        return "ERR";
    } else if (strncmp(instruction, STATUS_OK, CMD_LEN) == 0) {
        int fd = open(localFilePath, O_RDONLY, 0600);
        
        if (fd == -1) {
            perror("FOPEN");
            snprintf(instruction, INSTR_SIZE + 1, "%s", STATUS_ERR);
            sendData(localSocket, instruction, data, 0); // 07 OUT : ERROR
            DEBUG("=> ERROR");
        } else {
            snprintf(instruction, INSTR_SIZE + 1, "%s", STATUS_OK);
            sendData(localSocket, instruction, data, 0);  // 08 OUT : OK
            DEBUG("=> OK");

            recvData(localSocket, instruction, data);   // 08.1 IN  : OK
            ODEBUG("<= %s", instruction);
            
            pushFile(localSocket, fd);

            C_ALL(instruction, data);
            recvData(localSocket, instruction, data);   // 09 IN  : DONE
            ODEBUG("<= %s", instruction);
            
            printf("Transfer of %s finished.\n", _argv[1]);
        }
        close(fd);
    }

    C_ALL(instruction, data);
    free(localFilePath); free(instruction); free(data);
    
    return "builtin upload";
}

/**
 * TODO
 */
char* download(int localSocket, int *_argc, char *argv0, char **_argv) {
    if (*_argc != 2)
        (WARN("-download: invalid amount of arguments.", "ul <file> [<file new name>]"));
    
    unsigned char *instruction = malloc(INSTR_SIZE + 1), *data = malloc(BUFFER_SIZE + 1);
    
    char *localFilePath = malloc(FILENAME_MAX + 1);
    memset(localFilePath, 0, FILENAME_MAX + 1);
    snprintf(localFilePath, FILENAME_MAX, "%s/%s", getFilesFolder(argv0), _argv[1]);

    C_ALL(instruction, data); 
    snprintf(instruction, INSTR_SIZE + 1, "%s", CMD_DOWNLOAD);
    sendData(localSocket, instruction, data, 0); // 00 OUT : DOWNLOAD
    DEBUG("=> DOWNLOAD");

    C_ALL(instruction, data);
    recvData(localSocket, instruction, data);   // 00.1 IN : OK
    ODEBUG("<= %s", instruction);

    C_ALL(instruction, data);
    snprintf(instruction, INSTR_SIZE + 1, "%s", CMD_NAME);
    snprintf(data, BUFFER_SIZE + 1, "%s", _argv[1]);
    sendData(localSocket, instruction, data, strlen(_argv[1])); // 01 OUT : NAME <value>
    DEBUG("=> NAME");

    C_ALL(instruction, data);
    recvData(localSocket, instruction, data);       // 02|03|04|05 IN  : [ERROR|RESOURCE_IN_SOURCE|DENY|OK <size>]
    ODEBUG("<= %s", instruction);

    if (strncmp(instruction, STATUS_RESINUSE, CMD_LEN) == 0) {
        printColorized("The file is currently unavailable.", 32, 40, 0, 1);
        return "RIU";
    } else if (strncmp(instruction, STATUS_ERR, CMD_LEN) == 0) {
        printColorized("The file does not exist on the server.", 32, 40, 0, 1);
        return "ERR";
    } else if (strncmp(instruction, STATUS_DENY, CMD_LEN) == 0) {
        printColorized("The file cannot be read on the server.", 32, 40, 0, 1);
        return "DENY";
    } else {
        // Check file exists, prompt override / abort
        ull fileSize = strtoull(instruction + CMD_LEN + 1, NULL, 0);
        ODEBUG("FSIZE %llu", fileSize);

        if (isValidPath(localFilePath) == 1) {
            printf("`%s` already exists locally. OVERWRITE ? [Y/n] > ", _argv[1]);
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

            if (change == 0) {
                printf("Client chose to ABORT.\n");
                snprintf(instruction, INSTR_SIZE + 1, "%s", STATUS_ABORT);
                sendData(localSocket, instruction, data, 0);    // 06 OUT : ABORT
                DEBUG("=> ABORT");

                C_ALL(instruction, data);
                free(localFilePath); free(instruction); free(data);
                return "NO OVERWRITE EXIT";
            }
        }

        C_ALL(instruction, data);
        remove(localFilePath);
        int fd = open(localFilePath, O_CREAT | O_RDWR, 0600);

        if (fd == -1 || isLocked(localFilePath)) {
            printf("File Innacessible.\n");
            snprintf(instruction, INSTR_SIZE + 1, "%s", STATUS_DENY);
            sendData(localSocket, instruction, data, 0);    // 07 OUT : DENY
            DEBUG("=> DENY");

            C_ALL(instruction, data);
            free(localFilePath); free(instruction); free(data);
            return "FILE INNACESSIBLE";
        } else {
            
            lockFile(localFilePath);

            snprintf(instruction, INSTR_SIZE + 1, "%s", STATUS_OK);
            sendData(localSocket, instruction, data, 0);    // 08 OUT : OK
            DEBUG("=> OK");
            ODEBUG("FSIZE %llu", fileSize);
            DEBUG("PULL");
            pullFile(localSocket, fd, fileSize);

            C_ALL(instruction, data);
            snprintf(instruction, INSTR_SIZE + 1, "%s", STATUS_DONE);
            sendData(localSocket, instruction, data, 0);    // 09 OUT : DONE
            DEBUG("=> DONE");

            unlockFile(localFilePath);
        }

        close(fd);
    }    

    C_ALL(instruction, data); memset(localFilePath, 0, FILENAME_MAX + 1);
    free(instruction); free(data); free(localFilePath);
    
    return "builtin download";
}

char *deleteFile(int localSocket, int *_argc, char *argv0, char **_argv) {
    if (*_argc != 3)
        (WARN("-delete: invalid amount of arguments.", "del (l: local|d: distant) <file name>"));

    if (strncmp(_argv[1], "l", 1) == 0 || strncmp(_argv[1], "L", 1) == 0) {
        char *localFilePath = malloc(FILENAME_MAX + 1);
        memset(localFilePath, 0, FILENAME_MAX + 1);

        snprintf(localFilePath, FILENAME_MAX, "%s/%s", getFilesFolder(argv0), _argv[2]);

        if (isValidPath(localFilePath) == 1) {
            if (remove(localFilePath) == -1) {
                printColorized("`", 32, 40, 0, 0); printColorized(_argv[2], 32, 40, 0, 0); printColorized("`", 32, 40, 0, 0); printColorized(" could not be deleted.", 32, 40, 0, 1); 
            } else {
                printColorized("`", 32, 40, 0, 0); printColorized(_argv[2], 32, 40, 0, 0); printColorized("`", 32, 40, 0, 0); printColorized(" successfully deleted.", 32, 40, 0, 1); 
            }
        } else {
            printColorized("`", 32, 40, 0, 0); printColorized(_argv[2], 32, 40, 0, 0); printColorized("`", 32, 40, 0, 0); printColorized(" does not exist on the server.", 32, 40, 0, 1); 
        }

        free(localFilePath);
    } else if (strncmp(_argv[1], "d", 1) == 0 || strncmp(_argv[1], "D", 1) == 0) {
        unsigned char *instruction = malloc(INSTR_SIZE + 1), *data = malloc(BUFFER_SIZE + 1);
        C_ALL(instruction, data);

        snprintf(instruction, INSTR_SIZE + 1, "%s", CMD_DELETE);
        sendData(localSocket, instruction, data, 0);            // 00 OUT : DELETE
        DEBUG("=> DELETE");

        C_ALL(instruction, data);
        recvData(localSocket, instruction, data);               // 00.1 IN  : OK
        ODEBUG("<= %s", instruction);

        C_ALL(instruction, data);
        snprintf(instruction, INSTR_SIZE + 1, "%s", CMD_NAME);
        snprintf(data, BUFFER_SIZE + 1, "%s", _argv[2]);
        sendData(localSocket, instruction, data, strlen(data));  // 01 OUT : NAME <value>
        DEBUG("=> NAME");

        C_ALL(instruction, data);
        recvData(localSocket, instruction, data);               // [02|03|04|05] IN  : [ERROR|RIU|DENY|OK]
        ODEBUG("<= %s", instruction);

        if (strncmp(instruction, STATUS_ERR, CMD_LEN) == 0) {
            printColorized("`", 32, 40, 0, 0); printColorized(_argv[2], 32, 40, 0, 0); printColorized("`", 32, 40, 0, 0); printColorized(" does not exist on the server.", 32, 40, 0, 1); 
        } else if (strncmp(instruction, STATUS_DENY, CMD_LEN) == 0) {
            printColorized("`", 32, 40, 0, 0); printColorized(_argv[2], 32, 40, 0, 0); printColorized("`", 32, 40, 0, 0); printColorized(" could not be deleted from the server.", 32, 40, 0, 1); 
        } else if (strncmp(instruction, STATUS_RESINUSE, CMD_LEN) == 0) {
            printColorized("`", 32, 40, 0, 0); printColorized(_argv[2], 32, 40, 0, 0); printColorized("`", 32, 40, 0, 0); printColorized(" is currently in use on the server.", 32, 40, 0, 1); 
        } else {
            printColorized("`", 92, 40, 0, 0); printColorized(_argv[2], 92, 40, 0, 0); printColorized("`", 92, 40, 0, 0); printColorized(" successfully deleted from the server.", 92, 40, 0, 1);
        }
        
        C_ALL(instruction, data);
        free(instruction); free(data);
    } else
        (WARN("-delete: Invalid locale specifier.", "del (l: local|d: distant) <file name>"));
    
    return "DELETE FILE";
}

char* getHelp(int localSocket, int *_argc, char *argv0, char **_argv) {
    printf("Built-ins :\n- help\n- list\n- ul <file> [...]\n- dl <file> [...]\n- del (l: local|d: distant) <file name>\n");
    return "NO";
}

/** Define the functions */
char* (*builtin[]) (int, int *, char*, char **) = {
    &list,
    &upload,
    &download,
    &deleteFile,
    &getHelp
};

/** Define the builtins */
char *builtins[] = {
    "list",
    "ul",
    "dl",
    "del",
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