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
#include <time.h>

#include "Server.h"

/**
 * Main host function.
 * On success, returns an open and valid file descriptor to communicate with a client.
 * On error, returns RETURN_ERROR or RETURN_REBOOT.
 */
int host();

/**
 * Main client function.
 * On success, returns an open and valid file descriptor to communicate with an host.
 * On error, returns RETURN_ERROR or RETURN_REBOOT.
 */
int client();

#endif
