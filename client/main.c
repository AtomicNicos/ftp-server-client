#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "../common/utils.h"

int sendPacket(int server, int maxSize, char* fmt, ...) {
    char *buffer = malloc(maxSize);
    va_list args;
    va_start(args, fmt);
    vsprintf(buffer, fmt, args);
    va_end(args);

    int nsent = send(server, buffer, strlen(buffer), 0);

    free(buffer);
    return nsent;
}

int receivePacket(int server, int maxSize, char* buffer) {

}

void handShake(int server) {

}

int main(int argc, char **argv) {
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    int sock, server;

    printf("START\n");

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        FAIL_SUCCESFULLY("Socket could not be created\n");

    if (inet_pton(AF_INET, SERVER_IP, &(serv_addr.sin_addr)) < 0)    // Setup socket connection parameters.
        FAIL_SUCCESFULLY("INET_ERROR\n");
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

	if ((server = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        FAIL_SUCCESFULLY("CANNOT CREATE SOCKET\n");

    if (connect(server, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) // Connect to the socket.
        FAIL_SUCCESFULLY("SOCKET CONNECTION ERROR\n");
    
    printf("SENT %d bytes\n", sendPacket(server, BUFFER_SIZE, "%s", "TEST"));
    sleep(1);
    printf("SENT %d bytes\n", sendPacket(server, BUFFER_SIZE, "%s %d", "TEST2", 42));

    close(sock);
    close(server);
    exit(EXIT_SUCCESS);
}