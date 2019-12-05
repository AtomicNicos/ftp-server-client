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
#include "lineReader.h"
#include "builtins.h"

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
    crcInit();

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    int sock, server;

    printf("START\n");
    printf("%d\n", computeCRC("TEST", 16));


    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        FAIL_SUCCESFULLY("Socket could not be created\n");

    char *serverAddress = SERVER_IP;//getLine();
    if (inet_pton(AF_INET, serverAddress, &(serv_addr.sin_addr)) < 0)    // Setup socket connection parameters.
        FAIL_SUCCESFULLY("INET_ERROR\n");
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

	if ((server = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        FAIL_SUCCESFULLY("CANNOT CREATE SOCKET\n");

    if (connect(server, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) // Connect to the socket.
        FAIL_SUCCESFULLY("SOCKET CONNECTION ERROR\n");

    char *env_user = getenv("USER");    // Get environment values.
    char *distant_cwd = malloc(FILENAME_MAX);
    memset(distant_cwd, 0, FILENAME_MAX);
    strncat(distant_cwd, "~", FILENAME_MAX);

    int clientShouldRun = 1;
    do {
        print_prompt(env_user, serverAddress, distant_cwd);

        char *line = getLine(); // Get user input.

        int *_argc = malloc(sizeof(int));
        char **_argv = splitLine(line, _argc);  // Generate an asymmetrical component holder of vacuous contents or some such bs.
        printf("ARGC %i\n", *_argc);

        if (*_argc > 0) {
            char * builtinCommand = executeBuiltin(_argc, _argv);
            if (builtinCommand == NULL) {        // Not a builtin
                if (*_argc == 1 && strncmp("exit", _argv[0], 4) == 0) {
                    if (strlen(_argv[0]) == 4) {
                        
                        // EXIT handShake
                        printf("Bye\n");    // Leave.
                        
                        clientShouldRun = 0;
                    } else 
                        fprintf(stderr, "\nCommand '%s' not found, did you mean:\n\n  command 'exit' from deb trolling-you.\n\nTry: sudo apt install <deb name>\n\n", _argv[0]);
                } else {
                    fprintf(stderr, "\nCommand '%s' not found.\n\n", _argv[0]);
                }
            } else if (strlen(builtinCommand) == 2 && strncmp(builtinCommand, "NO", 2) == 2) {
                printf("%s\n", builtinCommand);
            } else 
                printf("%s\n", builtinCommand);
         }

    } while (clientShouldRun == 1);
    
    printf("SENT %d bytes\n", sendPacket(server, BUFFER_SIZE, "%s", "TEST"));
    sleep(1);
    printf("SENT %d bytes\n", sendPacket(server, BUFFER_SIZE, "%s %d", "TEST2", 42));

    close(sock);
    close(server);
    exit(EXIT_SUCCESS);
}