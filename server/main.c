#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "../common/utils.h"
#include "../common/crc.h"

//\033[%i;%i;%im%s\033[0m

int main(int argc, char **argv) {
    crcInit();

    struct sockaddr_in serv_addr, client_addr;
    unsigned int client_addr_length = sizeof(client_addr);
    int sock, connectionCount = 0;
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
    int *_bytes = malloc(sizeof(int)), *_contentSize = malloc(sizeof(int));
    int *_status = malloc(sizeof(int));

    do {
        client = accept(sock, (struct sockaddr *) &client_addr, &client_addr_length);
        connectionCount += 1;

        if (client < 0)
            printColorized("Client connection refused", 31, 40, 0, 1);
        else {
            char *client_origin = malloc(INET_ADDRSTRLEN);
            inet_ntop(AF_INET, &(client_addr.sin_addr), client_origin, client_addr_length);
            char *addr = malloc(8);
            snprintf(addr, 8, "%d", client_addr.sin_port);

            printf("Connect %i from [IP:PORT] [", connectionCount); printColorized(client_origin, 32, 40, 0, 0); printf(":"); printColorized(addr, 34, 40, 0, 0); printf("]\n");

            free(client_origin);
            free(addr);
        }

        
        sendMessage(client, "ERNUCNFOWBGYLRFCCRWFHKRYBYBKTGFNMYXZQSJYVKOZVFYXYHTOSYIKEGNIRBWZKLSPVGGGBCKBETEGQYYYSTTFYLVPGXQJGZZCDUIHNTVBBKTHPMZKSQGCZSLCFRKENUNSFISGQJLYDHZEVPLORVKLNJBGNEEEBXVKHJDCTGQOMRLZJOLYXBVVFZJIGZIWOHEGTXRUWDMMLTFWIDLLFQJMFOQWJNPSNCCDBMJPEYNXQFKSJQGTJOZGWMMRJMCRXCFBHXJRNSDXQQVXMMDJYEEIHPRPSLDDWRLUXBJUIRUSZGLOMNDWHBEZNTUDYIJXWTGZKGDWDZLROWMYMFZQDOLQVQCFQLCCRTPQCPOJXNEHSRLMKGFYUDZNVPDNRCYEEDNDJRBMRLZBPZFQSMCOHBUZJZKCEYFMZVKVYZITCEXUUWJGNIQJYDTVLQRJGWUKRZXXDKORKLULQBHUZKLZSNFCVQSGVKYHWPTWLTRXLLDEPCNNMXQVEUQVISHPEUFITZERDZEOYSZOXNXQISBPNCT");

        char *buffer = malloc(FRAME_SIZE + BUFFER_SIZE);
        memset(buffer, 0, BUFFER_SIZE);
        int clientConnected = 1;
        do {
            *_status = receivePacket(client, BUFFER_SIZE, _bytes, _contentSize, buffer);
            pprint(_bytes, _contentSize, _status, buffer, 0);

            if (*_bytes == 0) {
                printf("Received no bytes from the client.\n");
            }
            if (*_contentSize == 4 && strncmp(buffer, CMD_EXIT, 4) == 0) {
                printf("BYE\n");
                clientConnected = 0;
            }

        } while (clientConnected == 1);
        free(buffer);
        close(client);
    } while(programShouldRun == 1);

    sleep(1);
    printf("STOP\n");
    free(_bytes);
    free(_status);
    close(sock);
    exit(EXIT_SUCCESS);
}