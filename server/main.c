#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <signal.h>

#include "builtins.h"
#include "../common/utils.h"
#include "../common/fileHandler.h"
#include "../common/crc.h"

//\033[%i;%i;%im%s\033[0m

int programShouldRun = 1;
int clientConnected = 1;

/** @brief Handles signals.... 
 * @param signo The signal number.
 */
void signalHandler(int signo) {
    if (signo == SIGINT || signo == SIGTERM || signo == SIGQUIT || signo == SIGHUP) {
        clientConnected = 0;
        usleep(1);
        programShouldRun = 0;
    } else
        printf("WTF\n");
}

int main(int argc, char **argv) {
    // SETUP
    crcInit();

    struct sockaddr_in serv_addr, client_addr;
    unsigned int client_addr_length = sizeof(client_addr);
    int sock, connectionCount = 0;

    printf("START\n");

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
        printf("Listen for connections on a socket... OK\nServer listens for connections on a port 127.0.0.1:%d.\n", PORT);

    int client;

    struct sigaction action;    // Define signal handlers and sh*zz.
    memset(&action, 0, sizeof(action));
    action.sa_handler = signalHandler;
    sigaction(SIGINT, &action, NULL);

    // END SETUP
    
    do {
        client = accept(sock, (struct sockaddr *) &client_addr, &client_addr_length);
        connectionCount += 1;

        if (client < 0)
            printColorized("Client connection failed or broken", 31, 40, 0, 1);
        else {
            char *client_origin = malloc(INET_ADDRSTRLEN);
            inet_ntop(AF_INET, &(client_addr.sin_addr), client_origin, client_addr_length);
            char *addr = malloc(8);
            snprintf(addr, 8, "%d", client_addr.sin_port);

            printf("Connect %i from [IP:PORT] [", connectionCount); printColorized(client_origin, 32, 40, 0, 0); printf(":"); printColorized(addr, 34, 40, 0, 0); printf("]\n");

            free(client_origin);
            free(addr);
        }

        char *buffer = malloc(FRAME_SIZE + BUFFER_SIZE);
        do {
            char *instruction = malloc(INSTR_SIZE + 1);
            char *data = malloc(BUFFER_SIZE + 1);
            int size = recvData(client, instruction, data);
            printf("%ld TEXT |%s|\n", strlen(data), data);

            if (size == 0 && strncmp(instruction, CMD_EXIT, strlen(CMD_EXIT)) == 0) {
                printf("CLIENT EXITED\n");
                clientConnected = 0;
            } else if (size == 0 && strncmp(instruction, CMD_LIST, strlen(CMD_LIST)) == 0) {
                printf("CLIENT ASK FOR LIST\n");
                queryList(client, argv[0]);
            }else if (size == 0 && strncmp(instruction, CMD_UPLOAD, strlen(CMD_UPLOAD)) == 0) {
                printf("CLIENT WANTS TO UPLOAD\n");
                getFile(client, argv[0], instruction);
            }

            usleep(2);
            free(instruction);
            free(data);
        } while (clientConnected == 1);
        free(buffer);
        close(client);
    } while(programShouldRun == 1);

    sleep(1);
    printf("STOP\n");
    close(sock);
    exit(EXIT_SUCCESS);
}