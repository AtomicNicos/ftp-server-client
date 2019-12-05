#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "utils.h"

#define SEND(sock, buffer, max_size, fmt, ...) {\
memset(buffer, 0, max_size);\
snprintf(buffer, max_size, fmt, __VA_ARGS__);\
write(sock, buffer, strlen(buffer)); /* Write to the socket. */\
}

void sendData(int server, char* fmt, ...) {

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

    char *buffer = malloc(BUFFER_SIZE);
    snprintf(buffer, BUFFER_SIZE, "%s", "TEST");
    int nsent = send(server, buffer, strlen(buffer), 0);
    printf("SENT %d bytes\n", nsent);

    free(buffer);

    close(sock);
    close(server);
    exit(EXIT_SUCCESS);
}