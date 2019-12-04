#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "utils.h"

//\033[%i;%i;%im%s\033[0m

/** @brief Receives content from the socket, closes connection if unsuccessful.
 * @param sock The socket.
 * @param buffer The reception buffer (a char*).
 * @param max_size The maximum tolerated read size.
 * @param code_var The name of the variable containing the response code.
 * @param code_success The response code that equates to a successful transaction.
 * @param message_success The message to be displayed when the transaction is successful.
 */ 
#define RECEIVE(sock, buffer, max_size, code_var, code_success, message_success){\
printf("RESPONSE : ");\
memset(buffer, 0, max_size);\
read(sock, buffer, max_size); /* Get contents of the socket response. */\
printf("%s", buffer);\
if (code_var == code_success)\
    printf(message_success);\
else {\
    close(sock);\
    FAIL_SUCCESFULLY("THERE WAS AN ERROR\n\n"); /* EXIT ON FAILURE. */\
}\
}

/** @brief Pushes content to the socket.
 * @param sock The socket.
 * @param buffer The text to be sent (a char*).
 * @param max_size The maximum size of writing.
 * @param fmt The format of what should be written.
 * @param __VA_ARGS__ The format arguments.
 */ 
#define SEND(sock, buffer, max_size, fmt, ...) {\
memset(buffer, 0, max_size);\
snprintf(buffer, max_size, fmt, __VA_ARGS__);\
write(sock, buffer, strlen(buffer)); /* Write to the socket. */\
}

int main(int argc, char **argv) {
    struct sockaddr_in serv_addr, client_addr;
    unsigned int client_addr_length = sizeof(client_addr);
    int sock;

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
        printf("Listen for connections on a socket... OK\nServer listens for connections on a port %d.\n", PORT);

    int programShouldRun = 1;   // Non descript.
    int client;
    do {
        client = accept(sock, (struct sockaddr *) &client_addr, &client_addr_length);
        
        if (client < 0)
            printf("\033[31;40;0mClient connection refused\033[0m\n");
        else {
            char *client_origin = malloc(INET_ADDRSTRLEN);
            inet_ntop(AF_INET, &(client_addr.sin_addr), client_origin, client_addr_length);
            printf("New connection from [IP:PORT] [\033[32;40;0m%s\033[0m:\033[34;40;0m%d\033[0m]\n", client_origin, ntohs(client_addr.sin_port));
            free(client_origin);
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