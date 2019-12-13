#ifndef HEADER_COMMON_UTILS
#define HEADER_COMMON_UTILS

#include <stdio.h> /*   fprintf     */

typedef unsigned long long ull;
typedef signed long long sll;

#define DEBUGMODE 1

#define PORT 4242
#define BACKLOG 10
#define EXIT_FAILURE 1
#define SERVER_IP "127.0.0.1"

#define BUFFER_SIZE         512
#define INSTR_SIZE          32
#define DATA_OFFSET         (INSTR_SIZE + 4)    // + CRC
#define PACKET_SIZE         (BUFFER_SIZE + DATA_OFFSET)

#define CMD_LEN         4

#define CMD_EXIT        "EXIT"
#define CMD_UPLOAD      "UPLD"
#define CMD_DOWNLOAD    "DNLD"
#define CMD_BROADCAST   "ALRT"
#define CMD_NAME        "NAME"
#define CMD_LIST        "LIST"
#define CMD_OVERWRITE   "OVWR"
#define CMD_FILES       "FILS"
#define CMD_FILE        "FILE"
#define CMD_DELETE      "DELE"

#define STATUS_OK       "OKOK"
#define STATUS_DONE     "DONE"
#define STATUS_ERR      "ERRR"
#define STATUS_EMPTY    "EPTY"
#define STATUS_RESINUSE "REIU"
#define STATUS_DENY     "DENY"
#define STATUS_ABORT    "ABRT"

#define SPLIT_PLACES " \t"

#define FAIL_SUCCESFULLY(msg)       { fprintf(stderr, msg); exit(EXIT_FAILURE); }
#define FAIL_FSUCCESFULLY(msg, ...) { fprintf(stderr, msg, __VA_ARGS__); exit(EXIT_FAILURE); }

#define DEBUG(msg) { printColorized(("D: "), 95, 40, 0, 0); if((DEBUGMODE) == 1) printColorized((msg), 95, 40, 0, 1); }
#define ODEBUG(fmt, ...) { printColorized(("D: "), 95, 40, 0, 0); if((DEBUGMODE) == 1) { char *msg = malloc(256); snprintf(msg, 256, fmt, __VA_ARGS__); printColorized(msg, 95, 40, 0, 1); free(msg); }}
#define CDATA(data) { memset((data), 0, (BUFFER_SIZE + 1)); }
#define CINST(instruction) { memset((instruction), 0, (INSTR_SIZE + 1)); }
#define C_ALL(instruction, data) { CINST((instruction)); CDATA((data)); }

void printColorized(char *string, int ANSI_FGCOLOR, int ANSI_BGCOLOR, int ANSI_DECO, int newLine);

int sendData(int localSocket, unsigned char instruction[INSTR_SIZE], unsigned char data[BUFFER_SIZE], int contentLen);

int recvData(int localSocket, unsigned char instruction[INSTR_SIZE], unsigned char data[BUFFER_SIZE]);

char **splitLine(char *line, int *count, char *tokens);

char *getFilesFolder(char *argv0);

int pushFile(int localSocket, int fd);

int pullFile(int localSocket, int fd, ull fileSize);

#endif