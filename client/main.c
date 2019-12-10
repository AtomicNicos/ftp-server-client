#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "../common/utils.h"
#include "../common/crc.h"
#include "lineReader.h"
#include "builtins.h"

int server, clientShouldRun = 1;

/** @brief Handles signals.... 
 * @param signo The signal number.
 */
void signalHandler(int signo) {
    if (signo == SIGINT || signo == SIGTERM || signo == SIGQUIT || signo == SIGHUP) {
        sendData(server, CMD_EXIT, "");
        clientShouldRun = 0;
    } else
        printf("WTF\n");
}

int main(int argc, char **argv) {

    if (argc != 1)
        FAIL_SUCCESFULLY("TOO MANY parameters");
    crcInit();

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    int local_socket;

    printf("START\n");

    if ((local_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
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

    struct sigaction action;    // Define signal handlers and sh*zz.
    memset(&action, 0, sizeof(action));
    action.sa_handler = signalHandler;
    sigaction(SIGINT, &action, NULL);

    char *instruction = malloc(INSTR_SIZE + 1);
    char *data = malloc(BUFFER_SIZE + 1);
    //snprintf(instruction, INSTR_SIZE + 1, "%s", "HELPHELPHELPHELPHELPHELPHELPHELP");
    //snprintf(data, BUFFER_SIZE + 1, "%s", "ERNUCNFOWBGYLRFCCRWFHKRYBYBKTGFNMYXZQSJYVKOZVFYXYHTOSYIKEGNIRBWZKLSPVGGGBCKBETEGQYYYSTTFYLVPGXQJGZZCDUIHNTVBBKTHPMZKSQGCZSLCFRKENUNSFISGQJLYDHZEVPLORVKLNJBGNEEEBXVKHJDCTGQOMRLZJOLYXBVVFZJIGZIWOHEGTXRUWDMMLTFWIDLLFQJMFOQWJNPSNCCDBMJPEYNXQFKSJQGTJOZGWMMRJMCRXCFBHXJRNSDXQQVXMMDJYEEIHPRPSLDDWRLUXBJUIRUSZGLOMNDWHBEZNTUDYIJXWTGZKGDWDZLROWMYMFZQDOLQVQCFQLCCRTPQCPOJXNEHSRLMKGFYUDZNVPDNRCYEEDNDJRBMRLZBPZFQSMCOHBUZJZKCEYFMZVKVYZITCEXUUWJGNIQJYDTVLQRJGWUKRZXXDKORKLULQBHUZKLZSNFCVQSGVKYHWPTWLTRXLLDEPCNNMXQVEUQVISHPEUFITZERDZEOYSZOXNXQISBPNCT");

    //sendData(server, instruction, data);

    do {

        memset(instruction, 0, INSTR_SIZE + 1);
        memset(data, 0, BUFFER_SIZE + 1);

        printf("\n");

        print_prompt(env_user, serverAddress, distant_cwd);
        char *line = getLine(); // Get user input.
        int *_argc = malloc(sizeof(int));
        char **_argv = splitLine(line, _argc);  // Generate an asymmetrical component holder of vacuous contents or some such bs.
        
        //printf("ARGC %i\n", *_argc);

        if (*_argc > 0 && clientShouldRun == 1) {
            char * builtinCommand = executeBuiltin(server, _argc, _argv);
            if (builtinCommand == NULL) {        // Not a builtin
                if (*_argc == 1 && strncmp("exit", _argv[0], 4) == 0) {
                    if (strlen(_argv[0]) == 4) {
                        sendData(server, CMD_EXIT, "");
                        printf("Bye\n");    // Leave.
                        clientShouldRun = 0;
                    } else 
                        fprintf(stderr, "\nCommand '%s' not found, did you mean:\n\n  command 'exit' from deb trolling-you.\n\nTry: sudo apt install <deb name>\n\n", _argv[0]);
                } else {
                    fprintf(stderr, "\nCommand '%s' not found.\n\n", _argv[0]);
                }
            } else if (strlen(builtinCommand) == 2 && strncmp(builtinCommand, "NO", 2) == 0) {
                printf("ERROR %s\n", builtinCommand);
            }
        }
            
        // TODO Perhaps check server liveliness
    
        // Cleanup
        for (int i = 0; i < *_argc; i++) {
            _argv[i] = realloc(_argv[i], 0);
            _argv[i] = NULL;
        }

        memset(_argc, 0, sizeof(int));

        free(line);
        free(_argv);
        _argc = realloc(_argc, 0);
    

    } while (clientShouldRun == 1);

    free(instruction);
    free(data);

    close(local_socket);
    close(server);
    exit(EXIT_SUCCESS);
}