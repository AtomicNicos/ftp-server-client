#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../common/utils.h"
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

char* list(int *_argc, char **_argv) {
    if (*_argc != 1)
        (WARN("-list: too many arguments.", "list"));
    return "builtin list";
}

char* upload(int *_argc, char **_argv) {
    if (*_argc < 2)
        (WARN("-upload: insufficient arguments.", "ul <file> [...]"));
    return "builtin upload";
}

char* download(int *_argc, char **_argv) {
    if (*_argc < 2)
        (WARN("-download: insufficient arguments.", "dl <file> [...]"));
    return "builtin download";
}

char* changeDirectory(int *_argc, char **_argv) {
    if (*_argc > 2)
        (WARN("-cd: too many arguments.", "cd [<dir>]"));
    return "builtin cd";
}

char* getHelp(int *_argc, char **_argv) {
    printf("Built-ins :\n- help\n- cd [<dir>]\n- list\n- ul <file> [...]\n- dl <file> [...]\n");
    return "NO";
}

/** Define the functions */
char* (*builtin[]) (int *, char **) = {
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
char* executeBuiltin(int *_argc, char **_argv) {
    for (int i = 0; i < BUILTINS_COUNT; i++)
        if (strcmp(_argv[0], builtins[i]) == 0) 
            return (*builtin[i])(_argc, _argv);
    return NULL;
}