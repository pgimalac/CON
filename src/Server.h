#ifndef SERVER
#define SERVER

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NAME_LENGTH 21

#define TRUE 1
#define FALSE 0
#define EOS '\0'

// including the '\0' at the end
#define NAME_LENGTH 21
#define TOTAL_NUMBER_OF_HOSTS 5

/**
 * Ports.
 */
#define SERVER_PORT 8080
#define HOST_PORT 8888

/**
 * Return values of some functions.
 * RETURN_REBOOT isn't supposed to be returned, but is used inside some functions.
 */
#define RETURN_SUCCESS -1
#define RETURN_REBOOT -2
#define RETURN_ARG_ERROR -3
#define RETURN_ERROR -4

#define POLL_ERROR (POLLNVAL | POLLERR | POLLHUP)

#define FAT_BUFFER_SIZE (TOTAL_NUMBER_OF_HOSTS * (NAME_LENGTH + INET_ADDRSTRLEN) + 2 + sizeof(int))

#define WAIT_TIME 5000

/**
 * Codes used in messages.
 */
#define HOST_SERVER_QUIT 48
#define HOST_SERVER_NAME 49
#define CLIENT_SERVER_HOSTLIST 50
#define SERVER_CLIENT_STOP 51
#define SERVER_CLIENT_HOSTLIST 52
#define CLIENT_CLIENT_QUIT 53
#define CLIENT_CLIENT_MOVE 54

extern char buffer[FAT_BUFFER_SIZE];

struct sockaddr_in connect_to_client_addr;

typedef struct {
    char name[NAME_LENGTH];
    char ip[INET_ADDRSTRLEN];
} Host;

/**
 * Main server function.
 * If an error occurs, the server socket is closed and another is reopened.
 * Returns RETURN_REBOOT, RETURN_ERROR or RETURN_SUCCESS.
 */
int server();

/**
 * Returns a socket to communicate with the client.
 * On error, returns a negative number.
 * Called by the server and the host.
 */
int getTalkToClientSock(int);

/**
 * Returns a socket to communicate with the server.
 * On error, returns a negative number.
 * Called both by the host and by the client.
 * The string is an ip and the int a port.
 * If the given string is NULL, takes the ip of the server.
 */
int getTalkToServerSock(char*, int);

/**
 * if the given int matches POLLNVAL or POLLERR or POLLHUP, prints an error
 */
void printPollError(int, char*);

/**
 * If the given string's length is less than INET_ADDRSTRLEN
 * the function changes the server ip address
 */
void setServerIpAdress(char*);

#endif
