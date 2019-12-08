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
    printf("%s %i BYTES, CONTENT OF SIZE %i     STATUS : %i\n==%s==\n\n", (sent == 1) ? "SENT" : "RECVD", *bytes, *contentSize, *status, content);
}

int sendMessage(int localSocket, char *message, int packetSize, char *response) {
    int size = (int) (strlen(message) / COMMAND_SIZE) + 1;
    int *bytes = malloc(sizeof(int));
    int *contentSize = malloc(sizeof(int));
    int *status = malloc(sizeof(int));

    *status = sendPacket(localSocket, COMMAND_SIZE, bytes, "%s %s 0x%.16x", CMD_BROADCAST, "MSG", size);
    *status = receivePacket(localSocket, COMMAND_SIZE, bytes, response);
    pprint(bytes, status, status, response, 0);

    printf("THE RESPONSE WAS %s\n", response);
    if (strncmp(response, STATUS_OK, strlen(STATUS_OK)) == 0) {
        for (int i = 0; i < size; i++) {
            char *packet = malloc(packetSize + 1);
            snprintf(packet, COMMAND_SIZE + 1, "%s", message + (i * COMMAND_SIZE));
            *status = sendPacket(localSocket, COMMAND_SIZE + 1, bytes, "%s", packet);
            printf("  %i %s\n", i, packet);
            free(packet);

            char *response = malloc(COMMAND_SIZE + 1);
            *status = receivePacket(localSocket, COMMAND_SIZE, bytes, response);

            if (strncmp(response, STATUS_OK, strlen(STATUS_OK)) == 0) {
                printf("PACKET %d OK\n", i);
            } else if (strncmp(response, STATUS_ERR, strlen(STATUS_ERR)) == 0) {
                printf("PACKET ERR %d\n", i);
                i--;
            }

            free(response);
        }
    }
    
    free(bytes);
    free(contentSize);
    free(status);
    return 0;
}

/** @brief Sends a unary packet over the wire (Bootleg TCP : [packet_id][max_packet_id][packet_len][packet][CRC]).
 * @param localSocket   The local socket
 * @param packetNum     The number of the packet
 * @param maxPacketNum  The expected number of packets
 * @param packetSize    The size of the packet (512 for data / response, 64 for instructions / status)
 * @param bytes         An external variable (*ptr) that represents the amount of bytes that where sent
 * @param fmt           The format of the packet's contents
 * @param __VA_ARGS_    The packet's contents
 * @return 0 if OK else -1 an error was detected.
 */
/*int sendPacket(int localSocket, int packetNum, int maxPacketNum, int packetSize, int *bytes, char* fmt, ...) {
    char *buffer = malloc(packetSize);
    va_list args;
    va_start(args, fmt);
    vsprintf(buffer, fmt, args);
    va_end(args);

    //printf("NUM %d, PACKET TEXT : %ld | %s\n", (int) (packetSize - strlen(buffer)), strlen(buffer), buffer);

    char *amendedBuffer = malloc(2 * PACKET_INFO_SIZE + PACKET_SIZE_INDIC + packetSize + 1);
    snprintf(   amendedBuffer, 
                2 * PACKET_INFO_SIZE + PACKET_SIZE_INDIC + packetSize, 
                "%.4x%.4x%.3x%-*s",
                packetNum, maxPacketNum,
                (int) strlen(buffer),
                packetSize,
                buffer);


    int CRC = computeCRC(amendedBuffer, 2 * PACKET_INFO_SIZE + PACKET_SIZE_INDIC + packetSize);
    char *crcedBuffer = malloc(FRAME_SIZE + packetSize);
    snprintf(crcedBuffer, FRAME_SIZE + packetSize, "%s%.4x", amendedBuffer, CRC);

    printf("AMENDED SIZE %ld  %s\n", strlen(crcedBuffer), crcedBuffer);
    *bytes = send(localSocket, crcedBuffer, FRAME_SIZE + packetSize, 0);

    free(crcedBuffer);
    free(amendedBuffer);
    free(buffer);
    return (*bytes != packetSize + FRAME_SIZE) ? 0 : 1;
}*/

int sendPacket(int localSocket, int packetSize, int *bytes, char *fmt, ...) {
    char *buffer = malloc(packetSize);
    va_list args;
    va_start(args, fmt);
    vsprintf(buffer, fmt, args);
    va_end(args);

    char *amendedBuffer = malloc(PACKET_SIZE_INDIC + packetSize + 1);
    snprintf(   amendedBuffer, 
                PACKET_SIZE_INDIC + packetSize, 
                "%.3x%-*s",
                (int) strlen(buffer),
                packetSize,
                buffer);
    
    char *calcBuffer = malloc(PACKET_SIZE_INDIC + strlen(buffer) + 1);
    snprintf(   calcBuffer, PACKET_SIZE_INDIC + strlen(buffer) + 1, "%.3x%s",
                (int) strlen(buffer),
                buffer);

    printf("%ld %s\n", strlen(calcBuffer), calcBuffer);

    int CRC = computeCRC(calcBuffer, strlen(calcBuffer));
    char *crcedBuffer = malloc(PACKET_SIZE_INDIC + CRC_SIZE + packetSize + 1);
    snprintf(   crcedBuffer, 
                PACKET_SIZE_INDIC + CRC_SIZE + packetSize + 1, 
                "%.4x%s",
                CRC,
                amendedBuffer);

    printf("SENDING %s\n", crcedBuffer);
    *bytes = send(localSocket, crcedBuffer, CRC_SIZE + PACKET_SIZE_INDIC + packetSize, 0);
    int size = strlen(crcedBuffer);
    free(amendedBuffer);
    free(crcedBuffer);

    return size + 1;
}

char* receiveMessage(int localSocket, int packetSize, char *init) {
    int *nrecvd = malloc(sizeof(int));
    int *bytes = malloc(sizeof(int));
    int *status = malloc(sizeof(int));
    
    *status = sendPacket(localSocket, COMMAND_SIZE, bytes, "%s", STATUS_OK);

    printf("INIT WAS %s\n", init);
    long value = 0;
    for (int i = 0; i < strlen(init); i++)
        if (init[i] == '0' && value == 0)
            value = strtol(init + i, NULL, 0);
    
    printf ("VALUE %ld\n", value);
    char *message = malloc(packetSize * value + 1);
    char *packet = malloc(packetSize + 1);

    for (long i = 0; i < value; i++) {
        *status = receivePacket(localSocket, packetSize, bytes, packet);
        // CHECK CRC
        if (strlen(packet) > 0 && *status != -1) {
            printf("RECEIVING SOMETHING %ld %s\n", strlen(packet), packet);

            sendPacket(localSocket, COMMAND_SIZE, bytes, "%s", STATUS_OK);
            strncat(message, packet, packetSize);
        } else 
            i--;
    }
    free(packet);

    printf("MESSAGE IS %s\n", message);
    
    return message;
}

int receivePacket(int localSocket, int packetSize, int *bytes, char* buffer) {
    char *CRC = malloc(CRC_SIZE + 1), *contentSize = malloc(PACKET_SIZE_INDIC + 1),
         *packet = malloc(packetSize + CRC_SIZE + PACKET_SIZE_INDIC + 1), 
         *fullPacket = malloc(packetSize + PACKET_SIZE_INDIC + 1);
    
    *bytes = recv(localSocket, packet, packetSize + CRC_SIZE + PACKET_SIZE_INDIC, 0);
    
    snprintf(CRC, CRC_SIZE + 1, "%s", packet);
    
    snprintf(contentSize, PACKET_SIZE_INDIC + 1, "%s", packet + CRC_SIZE);
    int size = (int) strtol(contentSize, NULL, 16);
    
    snprintf(fullPacket, size + PACKET_SIZE_INDIC + 1, "%s", packet + CRC_SIZE);
    int arrivedCRC = (int) strtol(CRC, NULL, 16);
    int calculatedCRC = computeCRC(fullPacket, strlen(fullPacket));
/* 
    printf("\"002OK\" => %.4x\t", computeCRC("002OK", 5));

    printf("CRC %.4x %.4x %ld %s %s\n", arrivedCRC, calculatedCRC, strlen(fullPacket), fullPacket, packet);
 */
    snprintf(buffer, size + 1, "%s", packet + CRC_SIZE + PACKET_SIZE_INDIC);

    free(CRC); free(contentSize); free(packet); free(fullPacket);

    return (arrivedCRC != calculatedCRC) ? -1 : size;
}

/** @brief Receives a unary packet.
 * @param localSocket   The local socket
 * @param packetSize    The size of the packet
 * @param nrecvd        An external variable (*ptr) that represents the amount of bytes that where received.
 * @param _contentSize  An external variable (*ptr) that represents the size of the received content.
 * @param buffer        An external variable (*ptr) that will store the received content.
 * @return a status [0 | 0x10000 | 0x10001 | other] => [OK | Done | Excess | Error at packet number]
 */
/*int receivePacket(int localSocket, int packetSize, int *nrecvd, int *_contentSize, char* buffer) {
    char *packetNum = malloc(5), *packetMax = malloc(5),
         *contentSize = malloc(4), *_crc = malloc(5), 
         *packet = malloc(packetSize + FRAME_SIZE - 4), *fullPacket = malloc(packetSize + FRAME_SIZE);
    *nrecvd = recv(localSocket, fullPacket, packetSize + FRAME_SIZE, 0);
    
    snprintf(packet, packetSize + FRAME_SIZE - 3, "%s", fullPacket);
    snprintf(packetNum, 5, "%s", packet);
    snprintf(packetMax, 5, "%s", packet + 4);
    snprintf(contentSize, 4, "%s", packet + 8);
    snprintf(_crc, 5, "%s", fullPacket + packetSize + FRAME_SIZE - 5);

    int _packetNum = (int) strtol(packetNum, NULL, 16);
    int _packetMax = (int) strtol(packetMax, NULL, 16);
    *_contentSize = (int) strtol(contentSize, NULL, 16);

    snprintf(buffer, *_contentSize + 1, "%s", packet + 2 * PACKET_INFO_SIZE + PACKET_SIZE_INDIC);
    int arrivedCRC = (int) strtol(_crc, NULL, 16);
    int calculatedCRC = computeCRC(packet, packetSize + FRAME_SIZE - 4);
    
    free(packetNum); free(packetMax); free(contentSize); free(_crc); free(packet); free(fullPacket);

    printf("FULL %s ARR %d CALC %d", fullPacket, arrivedCRC, calculatedCRC);
    if (_packetNum > _packetMax)
        return TRANSMISSION_EXCESS;
    else if (arrivedCRC == calculatedCRC)
        return 0;
    else 
        return _packetNum;
}*/