#include "Network.h"

extern char buffer[FAT_BUFFER_SIZE];

int host() {
    struct pollfd poll_set[2] = {0};
    while ((poll_set[0].fd = getTalkToServerSock(NULL, SERVER_PORT)) ==
           RETURN_ERROR) {
        fprintf(
            stderr,
            "HOST : Unable to reach server. Will try again in 5 seconds.\n");
        sleep(WAIT_TIME);
    }

    poll_set[0].events = POLLIN;
    poll_set[1].fd = getTalkToClientSock(HOST_PORT);
    poll_set[1].events = POLLIN;

    char name[NAME_LENGTH + 1];
    name[0] = HOST_SERVER_NAME;

    int len;
    do {
        printf("Enter your nickname (between 2 and %d char) : ",
               NAME_LENGTH - 1);
        fgets(name + 1, NAME_LENGTH, stdin);
        len = strlen(name + 1);
        if (name[len] == '\n')
            name[len--] = EOS;
    } while (name[1] == EOS || name[2] == EOS);

    if (send(poll_set[0].fd, name, len + 1, MSG_NOSIGNAL) <= 0) {
        printf("Error sending the name to the server\n");
        return RETURN_ERROR;
    }

    int addrlen = sizeof(struct sockaddr_in), otherClientSocket;
    ssize_t length;

    do {
        printf("HOST : POLLing...\n");
        if (poll(poll_set, 2, -1) == -1) {
            perror("poll");
            return RETURN_ERROR;
        }

        // talk to server part
        if (poll_set[0].revents & POLL_ERROR) {
            printf("HOST_TO_SERVER : error with the server.\n");
            return RETURN_ERROR;
        } else if (poll_set[0].revents & POLLIN) {
            length = recv(poll_set[0].fd, buffer, 2, 0);

            if (length == -1) {
                perror("recv");
                close(poll_set[0].fd);
                close(poll_set[1].fd);
                return RETURN_ERROR;
            } else if (length == 0) {
                printf("HOST_TO_SERVER : server shut down.\n");
                close(poll_set[0].fd);
                close(poll_set[1].fd);
                return RETURN_ERROR;
            }

            printf("HOST_TO_SERVER : received %ld messages\n", length);
            // TODO ?
        }

        // talk to client part
        if (poll_set[1].revents & POLL_ERROR) {
            printPollError(poll_set[1].revents, "HOST_TO_CLIENT :");
            close(poll_set[0].fd);
            close(poll_set[1].fd);
            return RETURN_ERROR;
        } else if (poll_set[1].revents & POLLIN) {
            printf("HOST_TO_CLIENT : waiting for client connection.\n");
            if ((otherClientSocket = accept(
                     poll_set[1].fd, (struct sockaddr *)&connect_to_client_addr,
                     (socklen_t *)&addrlen)) == -1) {
                perror("HOST_TO_CLIENT : accept");
            } else {
                printf("Client found.\n");
                close(poll_set[0].fd);
                close(poll_set[1].fd);
                return otherClientSocket;
            }
        }
    } while (TRUE);
}

static void print_host_list(int host_number, char *buffer) {
    int len1, len2;

    for (int i = 1; i <= host_number; i++) {
        len1 = strlen(buffer) + 1;
        len2 = strlen(buffer + len1) + 1;
        printf("%d : %s - %s\n", i, buffer, buffer + len1);
        buffer += len1 + len2;
    }
}

int client() {
    int choice = RETURN_REBOOT;
    char buffer[FAT_BUFFER_SIZE];
    int talkToHostSock;
    do {
        printf("CLIENT : refreshing host list from server\n");
        int talkToServerSock;
        while ((talkToServerSock = getTalkToServerSock(NULL, SERVER_PORT)) ==
               RETURN_ERROR) {
            fprintf(stderr, "CLIENT : Unable to reach server. Will try again "
                            "in 5 seconds.\n");
            sleep(WAIT_TIME);
        }

        struct pollfd poll_fd;
        poll_fd.fd = talkToServerSock;
        poll_fd.events = POLLIN;

        printf("CLIENT : sending the refresh hostlist demand.\n");
        char client_server_hostlist = CLIENT_SERVER_HOSTLIST;
        if (send(talkToServerSock, &client_server_hostlist,
                 sizeof(client_server_hostlist), MSG_NOSIGNAL) == -1) {
            perror("CLIENT : send");
            return RETURN_ERROR;
        }

        printf("CLIENT : start waiting.\n");
        if (poll(&poll_fd, 1, WAIT_TIME) == -1) {
            perror("CLIENT : poll");
            return RETURN_ERROR;
        }

        int host_number = -1, valread;
        if (poll_fd.revents & POLLIN) {
            valread = recv(talkToServerSock, buffer, FAT_BUFFER_SIZE, 0);
            if (valread < 0) {
                perror("recv");
                close(talkToServerSock);
                return RETURN_ERROR;
            } else if (valread == 0) {
                printf("CLIENT error : server shut down.\n");
                close(talkToServerSock);
                return RETURN_ERROR;
            }

            if (buffer[0] != SERVER_CLIENT_HOSTLIST) {
                printf("CLIENT : unexpected message\n");
                close(talkToServerSock);
                continue;
            }
            memcpy(&host_number, buffer + 1, sizeof(int));
            printf("CLIENT : Host list (length %d, %d hosts) received from "
                   "server :\n",
                   valread - 1, host_number);
            print_host_list(host_number, buffer + 1 + sizeof(int));
        } else if (poll_fd.revents & POLL_ERROR) {
            fprintf(stderr, "CLIENT : error talking to the server.\n");
            return RETURN_ERROR;
        } else {
            printf("CLIENT : Revents : %d\n", poll_fd.revents);
            close(talkToServerSock);
            continue;
        }

        close(talkToServerSock);
        printf("CLIENT : Enter the number of the host, 0 to refresh or any "
               "other number to quit.\n");
        if (scanf("%d", &choice) < 1 || choice < 0)
            choice = RETURN_REBOOT;
        else if (choice != 0) {
            char *buffer2 = buffer + 1 + sizeof(int);
            for (int k = 2 * choice - 1; k > 0 && buffer2 - buffer <= valread;
                 k--)
                buffer2 += strlen(buffer2) + 1;

            // try to connect to the selected host
            printf("CLIENT : Connecting to the host...\n");

            if (strcmp(buffer2, "127.0.0.1") == 0)
                buffer2 = getServerIpAddress();
            else
                buffer2 = strdup(buffer2);

            if ((talkToHostSock = getTalkToServerSock(buffer2, HOST_PORT)) <
                0) {
                fprintf(stderr, "CLIENT : Error connecting to the host.\n");
                free(buffer2);
                choice = 0;
            } else {
                printf("%lu : Successful connection !\n",
                       (unsigned long)time(NULL));
                free(buffer2);
                return talkToHostSock;
            }
        } else
            choice = RETURN_REBOOT;
    } while (choice != RETURN_REBOOT);

    return choice;
}
