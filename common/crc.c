#include "crc.h"

crc  crcTable[256];

static unsigned long reflect(unsigned long data, unsigned char nBits) {
	unsigned long  reflection = 0x00000000;
	unsigned char  bit;

	// Reflect the data about the center bit.
	for (bit = 0; bit < nBits; ++bit) {
		// If the LSB bit is set, set the reflection of it.
		if (data & 0x01) 
			reflection |= (1 << ((nBits - 1) - bit));

		data = (data >> 1);
	}

	return (reflection);
}


void crcInit(void) {
    crc			   remainder;
	int			   dividend;
	unsigned char  bit;

    // Compute the remainder of each possible dividend.
    for (dividend = 0; dividend < 256; ++dividend)
    {
        //Start with the dividend followed by zeros.
        remainder = dividend << (WIDTH - 8);

        // Perform modulo-2 division, a bit at a time.
        for (bit = 8; bit > 0; --bit) {
            // Try to divide the current data bit.
            if (remainder & TOPBIT)
                remainder = (remainder << 1) ^ POLYNOMIAL;
            else 
                remainder = (remainder << 1);
        }

        // Store the result into the table.
        crcTable[dividend] = remainder;
    }
}

crc computeCRC(unsigned char const msg[], int size) {
    crc	           remainder = INITIAL_REMAINDER;
    unsigned char  data;
	int            byte;

    // Divide the message by the polynomial, a byte at a time.
    for (byte = 0; byte < size; ++byte) {
        data = REFLECT_DATA(msg[byte]) ^ (remainder >> (WIDTH - 8));
  		remainder = crcTable[data] ^ (remainder << 8);
    }

    // The final remainder is the CRC.
    return (REFLECT_REMAINDER(remainder) ^ FINAL_XOR_VALUE);
}