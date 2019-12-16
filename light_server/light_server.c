#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "../common/utils.h"
#include "light_server.h"
#include "llist.h"

/** @brief Handle a childs death nicely. *FBI stares*
 * @param signo The signal number.
 * @param s_info Siginfo structure, contains the pid.
 * @param no No.
*/
void signalHandler(int signo, siginfo_t *s_info, void *no) {
    if (signo == SIGTERM || signo == SIGQUIT || signo == SIGHUP || signo == SIGINT) {
        int num;
        while ((num = get_first()) && num != -1) {
            KILL(num);
            WAIT_FOR_DEATH(num);
            delete_item(num);
        }

        printf("Dying now !\n");
        
        KILL(getpid());
        WAIT_FOR_DEATH(getpid());
    } else
        printf("WTF\n");
}

void chldSignalHandler(int signo, siginfo_t *s_info, void *no) {
    if (signo == SIGCHLD && s_info) {
        ODEBUG("Server @[%d]+  done", s_info->si_pid);
        delete_item(s_info->si_pid);
        wait(NULL);
    }
}


int buildSocket() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock < 0)
        (FAIL_SUCCESFULLY("SOCKET CREATION FAILURE"));   
    
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);

    if (bind(sock, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0)
        FAIL_SUCCESFULLY("Socket binding failed.");
    
    if (listen(sock, BACKLOG) < 0)
        (FAIL_SUCCESFULLY("Deaf server\n"));
    else
        printf("Listen for connections on a socket... OK\nServer listens for connections on a port 127.0.0.1:%d.\n", PORT);
    
    return sock;
}

int main(int argc, char **argv) {
    char *rootPath = malloc(FILENAME_MAX + 1);
    snprintf(rootPath, FILENAME_MAX + 1, "%s%s", getenv("PWD"), argv[0] + 1);

    char *slash;
    int i = 0;
    while ((slash = strrchr(rootPath, '/')) && i < 2) {
        slash[0] = '\0';
        i++;
    }

    chdir(rootPath);

    struct sigaction action;
    memset(&action, 0, sizeof(action));
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    action.sa_sigaction = signalHandler;
    sigaction(SIGINT, &action, NULL);

    struct sigaction chldAction;    // Define signal handlers and sh*zz.
    memset(&chldAction, 0, sizeof(chldAction));
    chldAction.sa_flags = SA_SIGINFO;// | SA_RESTART; /* Child will alert. */
    chldAction.sa_sigaction = chldSignalHandler;
    sigaction(SIGCHLD, &chldAction, NULL);
    //sigaction(SIGINT, &chldAction, NULL);

    int socket = buildSocket();
    int serverRunning = 1;

    while (serverRunning == 1) {
        struct sockaddr_in client_addr;
        memset(&client_addr, 0, sizeof(client_addr));

        unsigned int addr_len = sizeof(client_addr);
        int client = accept(socket, (struct sockaddr *) &client_addr, &addr_len);

        printColorized("Light server launched", 32, 40, 0, 1);

        int pid = fork();
        if (pid > 0) {
            char *msg = malloc(256); snprintf(msg, 256, "Server instance starting on process ID [%d]", pid); printColorized(msg, 95, 40, 0, 1); free(msg);
            insert_at_last(pid);
        } else if (pid == 0) {
            if (client < 0) 
                FAIL_SUCCESFULLY("Failed to accept client connection\n");
            
            char **_argv = malloc(3 * sizeof(char *));
            _argv[0] = malloc(32);
            strncpy(_argv[0], "./server/ftp-server", 32);
            _argv[1] = malloc(32);
            snprintf(_argv[1], 32, "%i", client);
            _argv[2] = NULL;

            if (execvp(_argv[0], _argv))
                perror("Could not instantiate child");

            for (int i = 0; i < 3; i++)
                _argv[i] = realloc(_argv[i], 0);

            free(_argv);
            exit(EXIT_FAILURE);
        }
    }

    close(socket);
    return EXIT_SUCCESS;
}