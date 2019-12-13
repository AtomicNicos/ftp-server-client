#ifndef HEADER_CLIENT_BUILTINS
#define HEADER_CLIENT_BUILTINS

#include "../common/utils.h"

#define BUILTINS_COUNT 5

void print_prompt(char *user, char *host, char *cwd);

char* (*builtin[BUILTINS_COUNT]) (int, int *, char*, char **);

char *builtins[BUILTINS_COUNT];

char* executeBuiltin(int localSocket, int *_argc, char *argv0, char **_argv);

#define WARN(msg1, msg2) {\
printColorized((msg1), 31, 40, 1, 0);\
printf("  Syntax : ");\
printColorized((msg2), 34, 40, 0, 0);\
printf("\n");\
return "NO";\
}

#endif