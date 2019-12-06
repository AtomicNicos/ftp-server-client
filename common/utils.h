#ifndef HEADER_COMMON_UTILS
#define HEADER_COMMON_UTILS

#include <stdio.h> /*   fprintf     */

#define PORT 4242
#define BACKLOG 10
#define EXIT_FAILURE 1
#define SERVER_IP "127.0.0.1"

#define BUFFER_SIZE         512
#define COMMAND_SIZE        64
#define PROMPT_BUFFER_SIZE  256

#define TRANSMISSION_FINISHED   0x10000
#define TRANSMISSION_EXCESS     0x10001

#define CMD_ALIVE       "PING"
#define CMD_EXIT        "EXIT"
#define CMD_CHANGEDIR   "CD"
#define CMD_UPLOAD      "UL"
#define CMD_DOWNLOAD    "DL"
#define CMD_MKDIR       "MKDIR"
#define CMD_RENAME      "MV"
#define CMD_BROADCAST   "BRDCST"
#define STATUS_OK       "OK"
#define STATUS_ERR      "RESEND"
#define STATUS_RESINUSE "RIU"

#define FAIL_SUCCESFULLY(msg)       { fprintf(stderr, msg); exit(EXIT_FAILURE); }
#define FAIL_FSUCCESFULLY(msg, ...) { fprintf(stderr, msg, __VA_ARGS__); exit(EXIT_FAILURE); }

void printColorized(char *string, int ANSI_FGCOLOR, int ANSI_BGCOLOR, int ANSI_DECO, int newLine);

int sendMessage(int localSocket, char *message);

int sendPacket(int localSocket, int packetNum, int maxPacketNum, int packetSize, int *nsent, char* fmt, ...);

int receivePacket(int localSocket, int maxSize, int *nrecvd, int *contentSize, char *buffer);

void pprint(int *bytes, int *contentSize, int *status, char *content, int sent);


#endif