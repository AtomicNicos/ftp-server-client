#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "../common/utils.h"
#include "../common/fileHandler.h"
#include "builtins.h"

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

#define WARN(msg1, msg2) {\
printColorized((msg1), 31, 40, 1, 0);\
printf("  Syntax : ");\
printColorized((msg2), 34, 40, 0, 0);\
printf("\n");\
return "NO";\
}

char* list(int localSocket, int *_argc, char **_argv) {
    if (*_argc != 1)
        (WARN("-list: too many arguments.", "list"));

    char *instruction = malloc(INSTR_SIZE + 1); char *data = malloc(BUFFER_SIZE + 1);
    memset(instruction, 0, INSTR_SIZE + 1); memset(data, 0, BUFFER_SIZE + 1);
    sendData(localSocket, CMD_LIST, data);
    
    usleep(1);
    recvData(localSocket, instruction, data);

    long fCount = strtol(instruction + 6, NULL, 0);
    printf("%ld Remote Files\n", fCount);
    for (long i = 0; i < fCount; i++) {
        memset(instruction, 0, INSTR_SIZE + 1); memset(data, 0, BUFFER_SIZE + 1);
        usleep(1);
        recvData(localSocket, instruction, data);
        printf("  %ld => %s\n", i + 1, data);
    }
    
    free(instruction); free(data);
    return "builtin list";
}

char* upload(int localSocket, int *_argc, char **_argv) {
    if (*_argc != 2 && *_argc != 3)
        (WARN("-upload: invalid amount of arguments.", "ul <file> [<file new name>]"));

    int renameMode = (*_argc == 3) ? 1 : 0;

    char *file = malloc(FILENAME_MAX + 1);
    snprintf(file, FILENAME_MAX, "%s/client/~/%s", getenv("PWD"), _argv[1]);
    sll fileLength = getLength(file);

    if (fileLength >= 0)
        printf("FILE LEN %lld\n", fileLength);
    else 
        (WARN("-upload: specified file does not exist.", "ul <file> [<file new name>]"));

    char *instruction = malloc(INSTR_SIZE + 1); char *data = malloc(BUFFER_SIZE + 1);

    memset(instruction, 0, INSTR_SIZE + 1); memset(data, 0, BUFFER_SIZE + 1);
    snprintf(instruction, INSTR_SIZE + 1, "%s 0x%.17llx", CMD_UPLOAD, fileLength);
    sendData(localSocket, instruction, data);

    free(file);
    return "builtin upload";
}

char* download(int localSocket, int *_argc, char **_argv) {
    if (*_argc < 2)
        (WARN("-download: insufficient arguments.", "dl <file> [...]"));
    return "builtin download";
}

char* changeDirectory(int localSocket, int *_argc, char **_argv) {
    if (*_argc > 2)
        (WARN("-cd: too many arguments.", "cd [<dir>]"));
    return "builtin cd";
}

char* getHelp(int localSocket, int *_argc, char **_argv) {
    printf("Built-ins :\n- help\n- cd [<dir>]\n- list\n- ul <file> [...]\n- dl <file> [...]\n");
    return "NO";
}

/** Define the functions */
char* (*builtin[]) (int, int *, char **) = {
    &list,
    &upload,
    &download,
    &changeDirectory,
    &getHelp
};

/** Define the builtins */
char *builtins[] = {
    "list",
    "ul",
    "dl",
    "cd",
    "help"
};

/** @brief How intuitively well-named can a function be ?
 * @param _argv The arguments vector.
 */
char* executeBuiltin(int localSocket, int *_argc, char **_argv) {
    for (int i = 0; i < BUILTINS_COUNT; i++)
        if (strcmp(_argv[0], builtins[i]) == 0) 
            return (*builtin[i])(localSocket, _argc, _argv);
    return NULL;
}