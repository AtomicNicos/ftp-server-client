#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <zlib.h>

#include "../common/utils.h"

//\033[%i;%i;%im%s\033[0m

int main(int argc, char **argv) {
    crcInit();

    struct sockaddr_in serv_addr, client_addr;
    unsigned int client_addr_length = sizeof(client_addr);
    int sock;
    const char *pwd = getenv("PWD");

    printf("START\n");
    printf("%d\n", computeCRC("TEST", 16));

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        FAIL_SUCCESFULLY("Socket could not be created\n");

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = htons(INADDR_ANY);

    if (bind(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        FAIL_SUCCESFULLY("Socket could not be bound\n");

    if (listen(sock, BACKLOG) < 0)
        FAIL_SUCCESFULLY("Deaf server\n")
    else
        printf("Listen for connections on a socket... OK\nServer listens for connections on a port %d.\n", PORT);

    int programShouldRun = 1;   // Non descript.
    int client;

    do {
        client = accept(sock, (struct sockaddr *) &client_addr, &client_addr_length);
        
        if (client < 0)
            printColorized("Client connection refused", 31, 40, 0, 1);
        else {
            char *client_origin = malloc(INET_ADDRSTRLEN);
            inet_ntop(AF_INET, &(client_addr.sin_addr), client_origin, client_addr_length);
            char *addr = malloc(8);
            snprintf(addr, 8, "%d", client_addr.sin_port);

            printf("New connection from [IP:PORT] ["); printColorized(client_origin, 32, 40, 0, 0); printf(":"); printColorized(addr, 34, 40, 0, 0); printf("]\n");

            free(client_origin);
            free(addr);
        }

        char *buffer = malloc(BUFFER_SIZE);
        memset(buffer, 0, BUFFER_SIZE);
        
        int nread = 0, nreadIter = 0;
        nreadIter = recv(client, buffer, BUFFER_SIZE, 0);
        printf("RECVD %d BYTES\n%s\n", nreadIter, buffer);

        if (nreadIter == 0) {
            printf("Received no bytes from the client.\n");
        }

        free(buffer);
        close(client);
    } while(programShouldRun == 1);

    sleep(1);
    printf("STOP\n");
    close(sock);
    exit(EXIT_SUCCESS);
}