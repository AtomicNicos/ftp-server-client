#ifndef HEADER_SERVER_UTILS
#define HEADER_SERVER_UTILS

#include <stdio.h> /*   fprintf     */

#define PORT 4242
#define BACKLOG 10
#define BUFFER_SIZE 512
#define SERVER_IP "127.0.0.1"
#define EXIT_FAILURE 1

#define FAIL_SUCCESFULLY(msg)       { fprintf(stderr, msg); exit(EXIT_FAILURE); }
#define FAIL_FSUCCESFULLY(msg, ...) { fprintf(stderr, msg, __VA_ARGS__); exit(EXIT_FAILURE); }

#endif