#ifndef HEADER_COMMON_UTILS
#define HEADER_COMMON_UTILS

#include <stdio.h> /*   fprintf     */

typedef unsigned long long ull;
typedef signed long long sll;

#define PORT 4242
#define BACKLOG 10
#define EXIT_FAILURE 1
#define SERVER_IP "127.0.0.1"

#define BUFFER_SIZE         4096
#define DATA_OFFSET         40
#define INSTR_SIZE          32
#define PACKET_SIZE         (BUFFER_SIZE + DATA_OFFSET)

#define CMD_EXIT        "EXIT"
#define CMD_CHANGEDIR   "CD"
#define CMD_UPLOAD      "UL"
#define CMD_DOWNLOAD    "DL"
#define CMD_MKDIR       "MKDIR"
#define CMD_BROADCAST   "BRDCST"
#define CMD_LIST        "LIST"

#define CMD_PING        "PING"
#define CMD_PONG        "PONG"

#define STATUS_OK       "OK"
#define STATUS_DONE     "DONE"
#define STATUS_ERR      "ERROR"
#define STATUS_EMPTY    "EMPTY"
#define STATUS_RESINUSE "RIU"

#define SPLIT_PLACES " \t"

#define FAIL_SUCCESFULLY(msg)       { fprintf(stderr, msg); exit(EXIT_FAILURE); }
#define FAIL_FSUCCESFULLY(msg, ...) { fprintf(stderr, msg, __VA_ARGS__); exit(EXIT_FAILURE); }

void printColorized(char *string, int ANSI_FGCOLOR, int ANSI_BGCOLOR, int ANSI_DECO, int newLine);

void pprint(ull *bytes, int *contentSize, int *status, char *content, int sent);

int sendData(int localSocket, char instruction[INSTR_SIZE], char data[BUFFER_SIZE]);
int recvData(int localSocket, char instruction[INSTR_SIZE], char data[BUFFER_SIZE]);

char **splitLine(char *line, int *count, char *tokens);

#endif