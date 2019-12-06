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

void pprint(int *bytes, int *contentSize, int *status, char *content, int sent) {
    printf("%s %i BYTES, CONTENT OF SIZE %i     STATUS : %i\n=====START PACKET=====\n%s\n======END PACKET======\n", (sent == 1) ? "SENT" : "RECVD", *bytes, *contentSize, *status, content);
}

int sendMessage(int localSocket, char *message) {
    

    for (int i = 0; i < (int) (strlen(message) / COMMAND_SIZE) + 1; i++) {
        char *packet = malloc(COMMAND_SIZE + 1);
        snprintf(packet, COMMAND_SIZE + 1, "%s", message + (i * COMMAND_SIZE));
        free(packet);
    }

    return 0;
}

/** @brief Sends a unary packet over the wire (Bootleg TCP : [packet_id][max_packet_id][packet_len][packet][CRC]).
 * @param localSocket   The local socket
 * @param packetNum     The number of the packet
 * @param maxPacketNum  The expected number of packets
 * @param packetSize    The size of the packet (512 for data / response, 64 for instructions / status)
 * @param nsent         An external variable (*ptr) that represents the amount of bytes that where sent
 * @param fmt           The format of the packet's contents
 * @param __VA_ARGS_    The packet's contents
 * @return 0 if OK else -1 an error was detected.
 */
int sendPacket(int localSocket, int packetNum, int maxPacketNum, int packetSize, int *nsent, char* fmt, ...) {
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

    *nsent = send(localSocket, crcedBuffer, FRAME_SIZE + packetSize, 0);
    
    free(crcedBuffer);
    free(amendedBuffer);
    free(buffer);
    return (*nsent != packetSize + FRAME_SIZE) ? 0 : 1;
}

/** @brief Receives a unary packet.
 * @param localSocket   The local socket
 * @param packetSize    The size of the packet
 * @param nrecvd        An external variable (*ptr) that represents the amount of bytes that where received.
 * @param _contentSize  An external variable (*ptr) that represents the size of the received content.
 * @param buffer        An external variable (*ptr) that will store the received content.
 * @return a status [0 | 0x10000 | 0x10001 | other] => [OK | Done | Excess | Error at packet number]
 */
int receivePacket(int localSocket, int packetSize, int *nrecvd, int *_contentSize, char* buffer) {
    char *packetNum = malloc(5), *packetMax = malloc(5),
         *contentSize = malloc(4), *_crc = malloc(5), 
         *packet = malloc(packetSize + FRAME_SIZE - 4), *fullPacket = malloc(packetSize + FRAME_SIZE);
    *nrecvd = recv(localSocket, fullPacket, packetSize + FRAME_SIZE, 0);
    
    snprintf(packet, packetSize + FRAME_SIZE - 4, "%s", fullPacket);
    snprintf(packetNum, 5, "%s", packet);
    snprintf(packetMax, 5, "%s", packet + 4);
    snprintf(contentSize, 4, "%s", packet + 8);
    snprintf(_crc, 5, "%s", fullPacket + packetSize + FRAME_SIZE - 5);

    int _packetNum = (int) strtol(packetNum, NULL, 16);
    int _packetMax = (int) strtol(packetMax, NULL, 16);
    *_contentSize = (int) strtol(contentSize, NULL, 16);
    int contentOffset = packetSize - *_contentSize;

    snprintf(buffer, packetSize, "%s", packet + 2 * PACKET_INFO_SIZE + PACKET_SIZE_INDIC + contentOffset - 1);

    int arrivedCRC = (int) strtol(_crc, NULL, 16);
    int calculatedCRC = computeCRC(packet, packetSize + FRAME_SIZE - 4);
    
    free(packetNum); free(packetMax); free(contentSize); free(_crc); free(packet); free(fullPacket);

    if (_packetNum == _packetMax)
        return TRANSMISSION_FINISHED;
    else if (_packetNum > _packetMax)
        return TRANSMISSION_EXCESS;
    else if (arrivedCRC == calculatedCRC)
        return 0;
    else 
        return _packetNum;
}