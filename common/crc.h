#ifndef HEADER_COMMON_CRC
#define HEADER_COMMON_CRC

typedef unsigned short  crc;

#define WIDTH    (8 * sizeof(crc))
#define TOPBIT   (1 << (WIDTH - 1))

#define CRC_SIZE            4   // bytes
#define PACKET_INFO_SIZE    4   // bytes
#define PACKET_SIZE_INDIC   4   // bytes
#define FRAME_SIZE          (2 * (PACKET_INFO_SIZE) + (PACKET_SIZE_INDIC) + (CRC_SIZE))

#define POLYNOMIAL			0x8005
#define INITIAL_REMAINDER	0x0000
#define FINAL_XOR_VALUE		0x0000
#define REFLECT_DATA(X)			((unsigned char) reflect((X), 8))
#define REFLECT_REMAINDER(X)	((crc) reflect((X), WIDTH))

void  crcInit(void);

crc computeCRC(unsigned char const msg[], int size);

static unsigned long reflect(unsigned long data, unsigned char nBits);

#endif