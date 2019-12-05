#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>

#include "utils.h"
#include "crc.h"

/** Allows fancy color printouts to the console.
 * @param string    The string to funky print
 * @param ANSI_FG_COLOR A foreground ANSI setting
 * @param ANSI_BG_COLOR A background ANSI setting
 * @param ANSI_DECO A decoration (underline, bold, etc.) ANSI setting
 * @param newline Whether a newline should be printed at the end.
 */ 
void printColorized(char *string, int ANSI_FGCOLOR, int ANSI_BGCOLOR, int ANSI_DECO, int newLine) {
    printf("\033[%i;%i;%im%s\033[0m%s", ANSI_DECO, ANSI_BGCOLOR, ANSI_FGCOLOR, string, (newLine == 1) ? "\n\0" : "\0");
}

int sendPacket(int other_socket, int packetNum, int maxPacketNum, int packetSize, int *nsent, char* fmt, ...) {
    char *buffer = malloc(packetSize);
    va_list args;
    va_start(args, fmt);
    vsprintf(buffer, fmt, args);
    va_end(args);

    char *amendedBuffer = malloc(2 * PACKET_INFO_SIZE + PACKET_SIZE_INDIC + packetSize);
    snprintf(   amendedBuffer, 
                2 * PACKET_INFO_SIZE + PACKET_SIZE_INDIC + packetSize, 
                "%.4x%.4x%.3x%*s%s", 
                packetNum, maxPacketNum, 
                (int) strlen(buffer), (int) (BUFFER_SIZE - strlen(buffer)) - 1, 
                " ", 
                buffer);

    int CRC = computeCRC(amendedBuffer, 2 * PACKET_INFO_SIZE + PACKET_SIZE_INDIC + packetSize);
    char *crcedBuffer = malloc(FRAME_SIZE + packetSize);
    snprintf(crcedBuffer, FRAME_SIZE + packetSize, "%s%.4x", amendedBuffer, CRC);

    *nsent = send(other_socket, crcedBuffer, FRAME_SIZE + packetSize, 0);
    
    free(crcedBuffer);
    free(amendedBuffer);
    free(buffer);
    return (*nsent == -1) ? 0 : 1;
}

int receivePacket(int other_socket, int packetSize, int *nrecvd, char* buffer) {
    int nsent = recv(other_socket, buffer, FRAME_SIZE + packetSize, 0);
    return nsent;
}