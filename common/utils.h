#ifndef HEADER_COMMON_UTILS
#define HEADER_COMMON_UTILS

#include <stdio.h> /*   fprintf     */

#define PORT 4242
#define BACKLOG 10
#define EXIT_FAILURE 1
#define SERVER_IP "127.0.0.1"

#define BUFFER_SIZE 512
#define PROMPT_BUFFER_SIZE 256

#define FAIL_SUCCESFULLY(msg)       { fprintf(stderr, msg); exit(EXIT_FAILURE); }
#define FAIL_FSUCCESFULLY(msg, ...) { fprintf(stderr, msg, __VA_ARGS__); exit(EXIT_FAILURE); }

void printColorized(char *string, int ANSI_FGCOLOR, int ANSI_BGCOLOR, int ANSI_DECO, int newLine);

int sendPacket(int server, int packetNum, int maxPacketNum, int packetSize, int* nsent, char* fmt, ...);

int receivePacket(int server, int maxSize, int *nrecvd, char* buffer);

#endif