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
    
    if (strncmp(response, STATUS_OK, strlen(STATUS_OK)) == 0) {
        for (int i = 0; i < size; i++) {
            char *packet = malloc(packetSize + 1);
            snprintf(packet, COMMAND_SIZE + 1, "%s", message + (i * COMMAND_SIZE));
            *status = sendPacket(localSocket, COMMAND_SIZE + 1, bytes, "%s", packet);
            free(packet);

            char *response = malloc(COMMAND_SIZE + 1);
            *status = receivePacket(localSocket, COMMAND_SIZE, bytes, response);

            if (strncmp(response, STATUS_OK, strlen(STATUS_OK)) == 0) {
                printf("PACKET %d OK\n", i + 1);
            } else if (strncmp(response, STATUS_ERR, strlen(STATUS_ERR)) == 0) {
                printf("PACKET ERR %d\n", i + 1);
                i--;
            }

            free(response);
        }
    } else 
        printf("ERROR\n");
    
    free(bytes);
    free(contentSize);
    free(status);
    return 0;
}

/** @brief Sends a unary packet over the wire (Bootleg TCP : [packet_id][max_packet_id][packet_len][packet][CRC]).
 * @param localSocket   The local socket
 * @param packetSize    The size of the packet (512 for data / response, 64 for instructions / status)
 * @param bytes         An external variable (*ptr) that represents the amount of bytes that where sent
 * @param fmt           The format of the packet's contents
 * @param __VA_ARGS_    The packet's contents
 * @return 0 if OK else -1 an error was detected.
 */
int sendPacket(int localSocket, int packetSize, int *bytes, char *fmt, ...) {
    char *buffer = malloc(packetSize);
    va_list args;
    va_start(args, fmt);
    vsprintf(buffer, fmt, args);
    va_end(args);

    char *amendedBuffer = malloc(PACKET_SIZE_INDIC + packetSize + 1);
    snprintf(   amendedBuffer, 
                PACKET_SIZE_INDIC + packetSize, 
                "%.3x%s",
                (int) strlen(buffer),
                buffer);

    int CRC = computeCRC(amendedBuffer, strlen(amendedBuffer));
    char *crcedBuffer = malloc(PACKET_SIZE_INDIC + CRC_SIZE + packetSize + 1);
    snprintf(   crcedBuffer, 
                PACKET_SIZE_INDIC + CRC_SIZE + packetSize, 
                "%.4x%-*s",
                CRC,
                packetSize + PACKET_SIZE_INDIC,
                amendedBuffer);

    printf("SENDING |%c| |%s|\n", crcedBuffer[0], crcedBuffer);
    *bytes = send(localSocket, crcedBuffer, CRC_SIZE + PACKET_SIZE_INDIC + packetSize, 0);
    int size = strlen(crcedBuffer);

    free(amendedBuffer);
    free(crcedBuffer);

    return size;
}

char* receiveMessage(int localSocket, int packetSize, char *init) {
    int *bytes = malloc(sizeof(int));
    int *status = malloc(sizeof(int));
    
    *status = sendPacket(localSocket, COMMAND_SIZE, bytes, "%s", STATUS_OK);

    //printf("INIT WAS %s\n", init);
    long value = 0;
    for (int i = 0; i < strlen(init); i++)
        if (init[i] == '0' && value == 0)
            value = strtol(init + i, NULL, 0);
    
    char *message = malloc(packetSize * value + 1);
    char *packet = malloc(packetSize + 1);

    for (long i = 0; i < value; i++) {
        *status = receivePacket(localSocket, packetSize, bytes, packet);
        if (strlen(packet) > 0 && *status != -1) {
            sendPacket(localSocket, COMMAND_SIZE, bytes, "%s", STATUS_OK);
            strncat(message, packet, packetSize);
        } else if (strlen(packet) > 0 && *status == -1) {
            sendPacket(localSocket, COMMAND_SIZE, bytes, "%s", STATUS_ERR);
        } else
            i--;
    }
    free(packet);

    printf("MESSAGE IS %s\n", message);
    
    return message;
}

/** @brief Receives a unary packet.
 * @param localSocket   The local socket
 * @param packetSize    The size of the packet
 * @param bytes        An external variable (*ptr) that represents the amount of bytes that where received.
 * @param buffer        An external variable (*ptr) that will store the received content.
 * @return a status [0 | 0x10000 | 0x10001 | other] => [OK | Done | Excess | Error at packet number]
 */
int receivePacket(int localSocket, int packetSize, int *bytes, char* buffer) {
    char *CRC = malloc(CRC_SIZE + 1), *contentSize = malloc(PACKET_SIZE_INDIC + 1),
         *packet = malloc(packetSize + CRC_SIZE + PACKET_SIZE_INDIC + 1), 
         *fullPacket = malloc(packetSize + PACKET_SIZE_INDIC + 1);

    memset(buffer, 0, packetSize);
    memset(CRC, 0, CRC_SIZE + 1); memset(contentSize, 0, PACKET_SIZE_INDIC + 1);
    memset(packet, 0, packetSize + CRC_SIZE + PACKET_SIZE_INDIC + 1); memset(fullPacket, 0, packetSize + PACKET_SIZE_INDIC + 1);
    
    *bytes = recv(localSocket, packet, packetSize + CRC_SIZE + PACKET_SIZE_INDIC, 0);
    printf("RECEIVED PACKET |%c| |%s|\n", packet[0], packet);

    snprintf(CRC, CRC_SIZE + 1, "%s", packet);
    snprintf(contentSize, PACKET_SIZE_INDIC + 1, "%s", packet + CRC_SIZE);
    int size = (int) strtol(contentSize, NULL, 16);
    
    snprintf(fullPacket, size + PACKET_SIZE_INDIC + 1, "%s", packet + CRC_SIZE);
    int arrivedCRC = (int) strtol(CRC, NULL, 16);
    int calculatedCRC = computeCRC(fullPacket, strlen(fullPacket));

    snprintf(buffer, size + 1, "%s", packet + CRC_SIZE + PACKET_SIZE_INDIC);
    
    if (size > 0 && arrivedCRC == calculatedCRC) 
        printf("BUFFER = |%s|\n", buffer);

    free(CRC); free(contentSize); free(packet); free(fullPacket);

    return (arrivedCRC != calculatedCRC) ? -1 : size;
}