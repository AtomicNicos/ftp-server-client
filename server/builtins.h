#ifndef HEADER_SERVER_BUILTINS
#define HEADER_SERVER_BUILTINS

#include "../common/utils.h"

void queryList(int localSocket, char *argv0);

void getFile(int localSocket, char *argv0, unsigned char init[INSTR_SIZE]);

#endif