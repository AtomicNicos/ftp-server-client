// TODO DOC
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

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

int sendData(int localSocket, unsigned char instruction[INSTR_SIZE + 1], unsigned char data[BUFFER_SIZE + 1], int contentLen) {
    if (strlen(instruction) == 0 && strlen(data) == 0)
        return -1;
    
    unsigned char amendedBuffer[INSTR_SIZE + BUFFER_SIZE];
    memset(&amendedBuffer, 0, INSTR_SIZE + BUFFER_SIZE);
    
    snprintf(   amendedBuffer, 
                INSTR_SIZE + BUFFER_SIZE + 1, 
                "%-*s%s",
                INSTR_SIZE, instruction, data);
    
    
    int CRC = computeCRC(amendedBuffer, strlen(amendedBuffer));
    unsigned char crcedBuffer[PACKET_SIZE + 1];
    snprintf(   crcedBuffer, 
                PACKET_SIZE + 1, 
                "%.4x%s",
                CRC,
                amendedBuffer);
    

    int size = write(localSocket, crcedBuffer, DATA_OFFSET + contentLen);
    
    if (size == 0 || size == -1)
        perror("SEND DATA");

    return size;
}

int recvData(int localSocket, unsigned char instruction[INSTR_SIZE + 1], unsigned char data[BUFFER_SIZE + 1]) {
    unsigned char buffer[PACKET_SIZE];
    memset(&buffer, 0, PACKET_SIZE);
    int size = read(localSocket, buffer, sizeof buffer);

    if (size == 0 || size == -1)
        perror("RECV DATA");

    unsigned char *CRC = malloc(CRC_SIZE + 1);
    memset(CRC, 0, CRC_SIZE + 1);

    snprintf(CRC, CRC_SIZE + 1, "%s", buffer);

    int arrivedCRC = (int) strtol(CRC, NULL, 16);
    int calculatedCRC = computeCRC(buffer + CRC_SIZE, size - DATA_OFFSET);

    if (arrivedCRC == 0)
        snprintf(instruction, INSTR_SIZE + 1, "%s", STATUS_EMPTY);
    else if (arrivedCRC != calculatedCRC)
        snprintf(instruction, INSTR_SIZE + 1, "%s", STATUS_ERR);
    
    snprintf(instruction, INSTR_SIZE + 1, "%s", buffer + CRC_SIZE);
    snprintf(data, BUFFER_SIZE + 1, "%s", buffer + DATA_OFFSET);

    free(CRC);
    return (strncmp(instruction, STATUS_ERR, strlen(STATUS_ERR)) == 0 || strncmp(instruction, STATUS_EMPTY, strlen(STATUS_EMPTY)) == 0) ? -1 : size - DATA_OFFSET;
}

/** @brief Tokenizes the user's input
 * @param str The users input.
 * @param count The amount of params, useful externally.
 * @param tokens The tokens to separate on { @def SPLIT_PLACES }
 * @return The different tokens.
 */
char **splitLine(char *str, int *count, char *tokens) {
	int c = 0, l = strlen(str);
	for (int i = 0; i < l; i++) // Count the number of spaces to define the size of our mock _argv.
		if (str[i] == ' ' && str[i + 1] && str[i + 1] != ' ')
			c += 1;
			
	int pos = 0;
	char **elements = malloc((c + 2) * sizeof(char*));		// Token holder
	memset(elements, 0, (c + 2) * sizeof(char*));
	char *element;

	if (!elements) 
        FAIL_SUCCESFULLY("mAllocation Error");	// Handle errors

	element = strtok(str, SPLIT_PLACES);	// Tokenize on flags
	while (element != NULL) {	// Iterate on tokens
		elements[pos] = (char*) malloc(FILENAME_MAX);
		memset(elements[pos], 0, FILENAME_MAX);
		strncpy(elements[pos], element, FILENAME_MAX);
		pos++;
		element = strtok(NULL, SPLIT_PLACES);
	}

	elements[pos] = NULL;

	*count = pos;	// External use
	return elements;
}

char* getFilesFolder(char *argv0) {
    char *dirPath = malloc(FILENAME_MAX + 1);
    char *execPath = malloc(FILENAME_MAX + 1);

    snprintf(execPath, FILENAME_MAX + 1, "%s", argv0 + 1);

    char *slash = strrchr(execPath, '/');
    if (slash)
        slash[0] = '\0';
    
    snprintf(dirPath, FILENAME_MAX + 1, "%s%s/files", getenv("PWD"), execPath);
    free(execPath);
    return dirPath;
}

//TODO DOC
/** @author Guillaume Chanel, Prof. at UNIGE, gives a C course where the RDRW part of this code is provided in the public domain.
 *  @brief Send a file over the wire.
 *  @param localSocket
 *  @param fd
 *  @returns
 */
int pushFile(int localSocket, int fd) {
    unsigned char data[BUFFER_SIZE];
    sll bytesRead;

    while (bytesRead = read(fd, data, sizeof data), bytesRead > 0) {
        char *data_ptr = data;
        sll nWritten; 
        do {
            nWritten = write(localSocket, data_ptr, bytesRead);
            if (nWritten >= 0) {
                bytesRead -= nWritten;
                data_ptr += nWritten;
            } else if (errno != EINTR) {
                perror("WRITE TO SOCKET");
                return -1;
            }
        } while (bytesRead > 0);
    }
    return bytesRead;
}

/** @author Guillaume Chanel, Prof. at UNIGE, gives a C course where the RDRW part of this code is provided in the public domain.
 *  TODO DOC
 */
int pullFile(int localSocket, int fd, ull fileSize) {
    unsigned char data[BUFFER_SIZE];
    ull nRead, amountRead = 0;

    while (nRead = read(localSocket, data, sizeof data), nRead > 0) {
        amountRead += nRead;
        char *data_ptr = data;
        ull nWritten; 

        if (amountRead >= fileSize)
            nRead -= (amountRead - fileSize);
        do {
            nWritten = write(fd, data_ptr, nRead);
            if (nWritten >= 0) {
                nRead -= nWritten;
                data_ptr += nWritten;
            } else if (errno != EINTR) {
                perror("READ FROM SOCKET");
                return -1;
            }
        } while (nRead > 0);

        if (amountRead >= fileSize) 
            break;
    }
    return nRead;
}