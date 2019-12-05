#ifndef HEADER_SERVER_UTILS
#define HEADER_SERVER_UTILS

#include <stdio.h> /*   fprintf     */

#define PORT 4242
#define BACKLOG 10
#define BUFFER_SIZE 512
#define EXIT_FAILURE 1
#define SERVER_IP "127.0.0.1"

#define FAIL_SUCCESFULLY(msg)       { fprintf(stderr, msg); exit(EXIT_FAILURE); }
#define FAIL_FSUCCESFULLY(msg, ...) { fprintf(stderr, msg, __VA_ARGS__); exit(EXIT_FAILURE); }

typedef unsigned short  crc;

#define WIDTH    (8 * sizeof(crc))
#define TOPBIT   (1 << (WIDTH - 1))

#define CRC_NAME			"CRC-16"
#define POLYNOMIAL			0x8005
#define INITIAL_REMAINDER	0x0000
#define FINAL_XOR_VALUE		0x0000
#define REFLECT_DATA(X)			((unsigned char) reflect((X), 8))
#define REFLECT_REMAINDER(X)	((crc) reflect((X), WIDTH))
#define CHECK_VALUE			0xBB3D

void  crcInit(void);
crc computeCRC(unsigned char const msg[], int size);
static unsigned long reflect(unsigned long data, unsigned char nBits);

#endif