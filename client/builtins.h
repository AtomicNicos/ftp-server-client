#ifndef HEADER_CLIENT_BUILTINS
#define HEADER_CLIENT_BUILTINS

#define BUILTINS_COUNT 5

char* (*builtin[BUILTINS_COUNT]) (int *, char **);

char *builtins[BUILTINS_COUNT];

#endif