#ifndef NETWORK_INCLUDE
#define NETWORK_INCLUDE

#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "Server.h"

/**
 * Main host function.
 * On success, returns an open and valid file descriptor to communicate with a
 * client. On error, returns RETURN_ERROR or RETURN_REBOOT.
 */
int host();

/**
 * Main client function.
 * On success, returns an open and valid file descriptor to communicate with an
 * host. On error, returns RETURN_ERROR or RETURN_REBOOT.
 */
int client();

#endif
