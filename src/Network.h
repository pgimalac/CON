#ifndef NETWORK_INCLUDE
#define NETWORK_INCLUDE

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define TRUE 1
#define FALSE 0
#define EOS '\0'

// including the '\0' at the end
#define NAME_LENGTH 21
#define TOTAL_NUMBER_OF_HOSTS 5
#define FAT_BUFFER_SIZE (TOTAL_NUMBER_OF_HOSTS * (NAME_LENGTH + INET_ADDRSTRLEN) + 2)

#define WAIT_TIME 5000

/**
 * Ports.
 */
#define SERVER_PORT 8080
#define HOST_PORT 8888

/**
 * Codes used in messages.
 */
#define CLIENT_HOSTLIST_REFRESH 0
#define CLIENT_QUIT -1
#define MOVE -2

/**
 * Codes used in messages.
 */
#define ACK '0'
#define HOST_SERVER_QUIT '1'
#define HOST_SERVER_NAME '2'
#define CLIENT_SERVER_HOSTLIST '3'
#define SERVER_CLIENT_STOP '8'
#define SERVER_CLIENT_HOSTLIST '9'

/**
 * Return values of some functions.
 * RETURN_REBOOT isn't supposed to be returned, but is used inside some functions.
 */
#define RETURN_SUCCESS -1
#define RETURN_REBOOT -2
#define RETURN_ARG_ERROR -3
#define RETURN_ERROR -4

#define POLL_ERROR (POLLNVAL | POLLERR | POLLHUP)

typedef struct {
	char name[NAME_LENGTH];
	char ip[INET_ADDRSTRLEN];
} Host;

/**
 * If the given string's length is less than INET_ADDRSTRLEN
 * the function changes the server ip address
 */
void setServerIpAdress(char*);

/**
 * if the given int matches POLLNVAL or POLLERR or POLLHUP, prints an error
 */
void printPollError(int, char*);

/**
 * Main host function.
 * On success, returns an open and valid file descriptor to communicate with a client.
 * On error, returns RETURN_ERROR.
 */
int clientHost();

/**
 * Main client function.
 * On success, returns an open and valid file descriptor to communicate with an host.
 * On error, returns RETURN_ERROR.
 */
int client();

/**
 * Main server function.
 * This function doesn't return. If an error occurs, the server socket is closed and another is reopened.
 */
int server();

#endif
