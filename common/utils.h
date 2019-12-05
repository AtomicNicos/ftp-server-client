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

typedef unsigned short  crc;

#define WIDTH    (8 * sizeof(crc))
#define TOPBIT   (1 << (WIDTH - 1))

#define CRC_SIZE            16
#define PACKET_INFO         4
#define POLYNOMIAL			0x8005
#define INITIAL_REMAINDER	0x0000
#define FINAL_XOR_VALUE		0x0000
#define REFLECT_DATA(X)			((unsigned char) reflect((X), 8))
#define REFLECT_REMAINDER(X)	((crc) reflect((X), WIDTH))

void  crcInit(void);

crc computeCRC(unsigned char const msg[], int size);

static unsigned long reflect(unsigned long data, unsigned char nBits);

void printColorized(char *string, int ANSI_FGCOLOR, int ANSI_BGCOLOR, int ANSI_DECO, int newLine);

#endif