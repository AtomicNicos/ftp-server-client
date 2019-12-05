#ifndef HEADER_CLIENT_BUILTINS
#define HEADER_CLIENT_BUILTINS

#define BUILTINS_COUNT 5

void print_prompt(char *user, char *host, char *cwd);

char* (*builtin[BUILTINS_COUNT]) (int *, char **);

char *builtins[BUILTINS_COUNT];

char* executeBuiltin(int *_argc, char **_argv);

#endif