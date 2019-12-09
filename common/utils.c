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

void pprint(ull *bytes, int *contentSize, int *status, char *content, int sent) {
    printf("%s %lld BYTES, CONTENT OF SIZE %i     STATUS : %i\n==%s==\n\n", (sent == 1) ? "SENT" : "RECVD", *bytes, *contentSize, *status, content);
}

int sendData(int localSocket, char instruction[INSTR_SIZE], char data[BUFFER_SIZE]) {
    char *amendedBuffer = malloc(PACKET_SIZE_INDIC + INSTR_SIZE + BUFFER_SIZE + 1);
    memset(amendedBuffer, 0, PACKET_SIZE_INDIC + INSTR_SIZE + BUFFER_SIZE + 1);
    
    snprintf(   amendedBuffer, 
                PACKET_SIZE_INDIC + INSTR_SIZE + BUFFER_SIZE + 1, 
                "%-*s%.4x%-*s",
                INSTR_SIZE, instruction,
                (int) strlen(data),
                BUFFER_SIZE, data);
    
    
    int CRC = computeCRC(amendedBuffer, strlen(amendedBuffer));
    char *crcedBuffer = malloc(PACKET_SIZE + 1);
    snprintf(   crcedBuffer, 
                PACKET_SIZE + 1, 
                "%.4x%s",
                CRC,
                amendedBuffer);

    printf("SENDING <%ld> |%s|\n", strlen(crcedBuffer), crcedBuffer);
    int size = send(localSocket, crcedBuffer, PACKET_SIZE, 0);

    free(amendedBuffer); free(crcedBuffer);

    return size;
}

int recvData(int localSocket, char instruction[INSTR_SIZE + 1], char data[BUFFER_SIZE + 1]) {
    char *buffer = malloc(PACKET_SIZE + 1);
    int size = recv(localSocket, buffer, PACKET_SIZE, 0);

    char *CRC = malloc(CRC_SIZE + 1), *contentSize = malloc(PACKET_SIZE_INDIC + 1);
    memset(CRC, 0, CRC_SIZE + 1); memset(contentSize, 0, PACKET_SIZE_INDIC + 1);

    snprintf(CRC, CRC_SIZE + 1, "%s", buffer);
    snprintf(contentSize, PACKET_SIZE_INDIC + 1, "%s", buffer + CRC_SIZE + INSTR_SIZE);

    long _contentSize = strtol(contentSize, NULL, 16);
    int arrivedCRC = (int) strtol(CRC, NULL, 16);
    int calculatedCRC = computeCRC(buffer + CRC_SIZE, INSTR_SIZE + PACKET_SIZE_INDIC + BUFFER_SIZE);

    printf("ARR %.4x <=> %.4x CALC\n", arrivedCRC, calculatedCRC);
    if (arrivedCRC != calculatedCRC) {
        snprintf(instruction, INSTR_SIZE, "%s", "ERROR");
        return 0;
    }
    
    snprintf(instruction, INSTR_SIZE + 1, "%s", buffer + CRC_SIZE);
    snprintf(data, _contentSize + 1, "%s", buffer + DATA_OFFSET);

    free(buffer); free(CRC); free(contentSize);
    return _contentSize;
}