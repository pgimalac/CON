#include "Network.h"

extern char buffer[FAT_BUFFER_SIZE];
char SERVER_IP_ADDRESS[INET_ADDRSTRLEN] = "127.0.0.1";
struct sockaddr_in connect_to_client_addr;

void setServerIpAdress(char* add){
    if (add == NULL) return;

    if (strlen(add) >= INET_ADDRSTRLEN)
        fprintf(stderr, "%s is too long to be a valid IPV4 adress.\n", add);
    else
        strcpy(SERVER_IP_ADDRESS, add);
}

void printPollError(int revents, char* st){
    if (revents & POLLNVAL)
        fprintf(stderr, "%s POLLNVAL error with the socket (invalid socket)", st);
    if (revents & POLLHUP)
        fprintf(stderr, "%s POLLHUP error with the socket (network error)", st);
    if (revents & POLLERR)
        fprintf(stderr, "%s POLLERR error with the socket (network error or player left)", st);
}

/**
 * Copy source into dest and returns a pointer to the end of dest
 * (The first character after the end of dest)
 */
static char* strdcpy(char* dest, char* source){
    do {
        *dest = *source;
        dest++;
    } while (*source++);
    return dest;
}

/**
 * Returns a socket to communicate with the server.
 * On error, returns RETURN_ERROR.
 * Called both by the host and by the client.
 */
static int getTalkToServerSock(char* server_ip, int port){
    if (server_ip == NULL || strlen(server_ip) < 7) {
        printf("Enter server ip : \n");
        server_ip = fgets(server_ip, INET_ADDRSTRLEN, stdin);
        int length = strlen(server_ip);
        if (length != 0 && server_ip[length - 1] == '\n') server_ip[length - 1] = EOS;
    }
    printf("getTalkToServerSock server ip is %s\n", server_ip);

    struct sockaddr_in connect_to_server_addr = {0};
    int talkToServerSock;

    // creating the socket used to communicate with the server
    if ((talkToServerSock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("getTalkToServerSock socket");
        return RETURN_ERROR;
    }

    connect_to_server_addr.sin_family = AF_INET;
    connect_to_server_addr.sin_port = htons(port);

    short ret = inet_pton(AF_INET, server_ip, &connect_to_server_addr.sin_addr);
    if (ret == 0) {
        fprintf(stderr, "Invalid address/ Address not supported");
        return RETURN_ERROR;
    }

    if (ret < 0) {
        perror("inet_pton");
        return RETURN_ERROR;
    }

    if (connect(talkToServerSock, (struct sockaddr*)&connect_to_server_addr, sizeof(connect_to_server_addr)) == -1) {
        perror("getTalkToServerSock connect");
        return RETURN_ERROR;
    }
    // end

    return talkToServerSock;
}

/**
 * Returns a socket to communicate with the client.
 * On error, returns -1.
 * Called by the server and the host.
 */
static int getTalkToClientSock(int port){
    int talkToClientSock = socket(AF_INET, SOCK_STREAM, 0);

    // creating the local server to talk with the client
    if (talkToClientSock < 0) {
        perror("talkToClientSock socket");
        return -1;
    }

    int opt = 1;
    if (setsockopt(talkToClientSock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) == -1) {
        perror("talkToClientSock setsockopt");
        return -1;
    }

    connect_to_client_addr.sin_family = AF_INET;
    connect_to_client_addr.sin_addr.s_addr = INADDR_ANY;
    connect_to_client_addr.sin_port = htons(port);

    if (bind(talkToClientSock, (struct sockaddr*)&connect_to_client_addr, sizeof(connect_to_client_addr)) < 0) {
        perror("talkToClientSock bind");
        return -1;
    }

    if (listen(talkToClientSock, 1) < 0){
        perror("talkToClientSock listen");
        return -1;
    }
    // end

    return talkToClientSock;
}

int clientHost (){
    struct pollfd poll_set[2] = {0};
    while ((poll_set[0].fd = getTalkToServerSock(SERVER_IP_ADDRESS, SERVER_PORT)) == RETURN_ERROR){
        fprintf(stderr, "HOST : Unable to reach server. Will try again in 5 seconds.\n");
        sleep(WAIT_TIME);
    }

    poll_set[0].events = POLLIN;
    poll_set[1].fd = getTalkToClientSock(HOST_PORT);
    poll_set[1].events = POLLIN;

    char name[NAME_LENGTH + 1];
    name[0] = HOST_SERVER_NAME;

    int len;
    do {
        printf("Enter your nickname (between 2 and %d char) : ", NAME_LENGTH - 1);
        fgets(name + 1, NAME_LENGTH, stdin);
        len = strlen(name + 1);
        if (name[len] == '\n')
            name[len--] = EOS;
    } while (name[1] == EOS || name[2] == EOS);

    if (send(poll_set[0].fd, name, len + 1, MSG_NOSIGNAL) <= 0){
        printf("Error sending the name to the server\n");
        return RETURN_ERROR;
    }

    int addrlen = sizeof(struct sockaddr_in), otherClientSocket = 0;
    ssize_t length;

    do{
        printf("HOST : POLLing...\n");
        poll(poll_set, 2, -1);

        // talk to server part
        if (poll_set[0].revents & POLL_ERROR){
            printf("HOST_TO_SERVER : error with the server.\n");
            if (poll_set[0].revents & (POLLNVAL | POLLERR))
                close(poll_set[0].fd);
            return RETURN_ERROR;
        } else if (poll_set[0].revents & POLLIN) {
            length = recv(poll_set[0].fd, buffer, 2, 0);

            if (length == -1) {
                perror("recv");
                close (poll_set[0].fd);
                return RETURN_ERROR;
            } else if (length == 0) {
                printf("HOST_TO_SERVER : server shut down.\n");
                close(poll_set[0].fd);
                return RETURN_ERROR;
            }

            printf("HOST_TO_SERVER : received %ld messages\n", length);
            // TODO
        }

        // talk to client part
        if (poll_set[1].revents & POLL_ERROR){
            printPollError(poll_set[1].revents, "HOST_TO_CLIENT :");
            if (poll_set[1].revents & (POLLNVAL | POLLERR))
                close(poll_set[1].fd);
            return RETURN_ERROR;
        } else if (poll_set[1].revents & POLLIN) {
            printf("HOST_TO_CLIENT : waiting for client connection.\n");
            if ((otherClientSocket = accept(poll_set[1].fd, (struct sockaddr *)&connect_to_client_addr, (socklen_t*)&addrlen)) == -1){
                perror("HOST_TO_CLIENT : error adding a client");
            } else {
                printf("Client found.\n");
                return poll_set[1].fd;
            }
        }
    } while (TRUE);
}

static void print_host_list(int host_number, char* buffer) {
    int len1, len2;

    for (int i = 1; i <= host_number; i++){
        len1 = strlen(buffer) + 1;
        len2 = strlen(buffer + len1) + 1;
        printf("%d : %s - %s\n", i, buffer, buffer + len1);
        buffer += len1 + len2;
    }
}

int client () {
    int choice = CLIENT_HOSTLIST_REFRESH;
    char buffer[FAT_BUFFER_SIZE];
    int talkToHostSock;
    do{
        printf("CLIENT : refreshing host list from server\n");
        int talkToServerSock;
        while ((talkToServerSock = getTalkToServerSock(SERVER_IP_ADDRESS, SERVER_PORT)) == RETURN_ERROR){
            fprintf(stderr, "CLIENT : Unable to reach server. Will try again in 5 seconds.\n");
            sleep(WAIT_TIME);
        }

        struct pollfd poll_fd;
        poll_fd.fd = talkToServerSock;
        poll_fd.events = POLLIN;

        printf("CLIENT : sending the refresh hostlist demand.\n");
        char client_server_hostlist = CLIENT_SERVER_HOSTLIST;
        if (send(talkToServerSock, &client_server_hostlist, sizeof(client_server_hostlist), MSG_NOSIGNAL) == -1){
            perror("CLIENT : send");
            exit(EXIT_FAILURE);
        }

        printf("CLIENT : start waiting.\n");
        if (poll(&poll_fd, 1, WAIT_TIME) == -1){
            perror("CLIENT : poll");
            exit(EXIT_FAILURE);
        }

        int host_number = -1, valread;
        if (poll_fd.revents & POLLIN){
            valread = recv(talkToServerSock, buffer, FAT_BUFFER_SIZE, 20);
            if (valread < 0){
                perror("recv");
                close(talkToServerSock);
                return RETURN_ERROR;
            } else if (valread == 0) {
                printf("CLIENT error : server shut down.\n");
                close(talkToServerSock);
                return RETURN_ERROR;
            }

            if (buffer[0] != SERVER_CLIENT_HOSTLIST){
                printf("CLIENT : unexpected message\n");
                close(talkToServerSock);
                continue;
            }
            sscanf(buffer + 1, "%4s", (char*)&host_number);
            printf("CLIENT : Host list (length %d, %d hosts) received from server :\n", valread - 1, host_number);
            print_host_list(host_number, buffer + 1 + sizeof(int));
        } else if (poll_fd.revents & POLL_ERROR) {
            fprintf(stderr, "CLIENT : error talking to the server.\n");
            close(talkToServerSock);
            return RETURN_ERROR;
        } else {
            printf("CLIENT : Revents : %d\n", poll_fd.revents);
            close(talkToServerSock);
            continue;
        }

        close(talkToServerSock);
        printf("CLIENT : Enter the number of the host, %d to refresh or any other number to quit.\n", CLIENT_HOSTLIST_REFRESH);
        if (scanf("%d", &choice) < 1 || (choice <= 0 && choice != CLIENT_HOSTLIST_REFRESH))
            choice = CLIENT_QUIT;
        else if (choice != CLIENT_HOSTLIST_REFRESH) {
            char* buffer2 = buffer + 1 + sizeof(int);
            for (int k = 2 * choice - 1; k > 0 && buffer2 - buffer <= valread; k--)
                buffer2 += strlen(buffer2) + 1;

            // try to connect to the selected host
            printf("CLIENT : Connecting to the host...\n");
            if ((talkToHostSock = getTalkToServerSock(buffer2, HOST_PORT)) < 0) {
                fprintf(stderr, "CLIENT : Error connecting to the host.\n");
                choice = CLIENT_HOSTLIST_REFRESH;
            } else {
                printf("Successful connection !\n");
                return talkToHostSock;
            }
        }
    } while (choice != CLIENT_QUIT);

    return RETURN_SUCCESS;
}

static void removeHost(int *number_of_hosts, struct pollfd poll_set[*number_of_hosts + 1], Host* waiting_hosts[*number_of_hosts + 1], int i){
    printf("SERVER : removing host\n");
    close(poll_set[i + 1].fd);
    free(waiting_hosts[i]);
    for (int j = i; j < *number_of_hosts; j ++){ // move the others
        waiting_hosts[i] = waiting_hosts[i + 1];
        poll_set[i + 1] = poll_set[i + 2];
    }
    --*number_of_hosts;
}

int server () {
    int talkToClientSock = getTalkToClientSock(SERVER_PORT), new_socket, valread;
    socklen_t addrlen = sizeof(connect_to_client_addr);

    Host *waiting_hosts[TOTAL_NUMBER_OF_HOSTS];
    struct pollfd poll_set[1 + TOTAL_NUMBER_OF_HOSTS] = {0};
    poll_set[0].fd = talkToClientSock;
    poll_set[0].events = POLLIN;
    int number_of_hosts = 0;

    do{
        printf("SERVER : POLLing.\n");
        poll(poll_set, number_of_hosts + 1, -1);

        if (poll_set[0].revents & POLL_ERROR){
            printPollError(poll_set[0].revents, "SERVER :");
            for (int i = 0; i < number_of_hosts + 1; i++)
                close(poll_set[i].fd);
            printf("Rebooting...\n");
            return RETURN_REBOOT;
        }

        if (number_of_hosts < TOTAL_NUMBER_OF_HOSTS && poll_set[0].revents & POLLIN){
            printf("SERVER : Adding a new host (now number %d)\n", number_of_hosts);
            if ((new_socket = accept(talkToClientSock, (struct sockaddr*)&connect_to_client_addr, &addrlen)) == -1){
                fprintf(stderr, "SERVER : error adding an host\n");
                continue;
            }
            waiting_hosts[number_of_hosts] = (Host*)calloc(1, sizeof(Host));
            inet_ntop(AF_INET, &connect_to_client_addr.sin_addr, waiting_hosts[number_of_hosts]->ip, INET_ADDRSTRLEN);

            number_of_hosts ++;

            poll_set[number_of_hosts].fd = new_socket;
            poll_set[number_of_hosts].events = POLLIN;
        }

        for (int i = 0; i < number_of_hosts; i++){
            if (poll_set[i + 1].revents & POLL_ERROR){
                removeHost(&number_of_hosts, poll_set, waiting_hosts, i);
            } else if (poll_set[i + 1].revents & POLLIN){
                valread = recv(poll_set[i + 1].fd, &buffer, FAT_BUFFER_SIZE, 0);

                if (valread <= 0){
                    removeHost(&number_of_hosts, poll_set, waiting_hosts, i);
                } else {
                    buffer[valread] = EOS;
                    printf("length of the message %d, message : \"%s\"\n", valread, buffer);
                    if (buffer[0] == HOST_SERVER_NAME){ // change the name of the host
                        printf("SERVER : changing an host's name.\n");
                        if (valread > NAME_LENGTH)
                            fprintf(stderr, "SERVER : too long name\n");
                        else {
                            strncpy(waiting_hosts[i]->name, buffer + 1, valread - 1);
                            printf("SERVER : the name of %d was changed to \"%s\"\n", poll_set[i + 1].fd, waiting_hosts[i]->name);
                        }
                    } else if (buffer[0] == HOST_SERVER_QUIT){ // the host left
                        printf("SERVER : An host is gone...\n");
                        removeHost(&number_of_hosts, poll_set, waiting_hosts, i);
                    } else if (buffer[0] == CLIENT_SERVER_HOSTLIST){ // a client asks the host list
                        printf("SERVER : a client asked the host list.\n");
                        buffer[0] = SERVER_CLIENT_HOSTLIST;
                        char* buffer2 = buffer + 1 + sizeof(int);
                        int nb = 0;
                        for (int j = 0; j < number_of_hosts; j ++){
                            printf("%d : %s (%s)\n", poll_set[j + 1].fd, waiting_hosts[j]->name, waiting_hosts[j]->ip);

                            if (waiting_hosts[j]->name[0] == EOS)
                                continue;

                            nb ++;
                            buffer2 = strdcpy(buffer2, waiting_hosts[j]->name);
                            buffer2 = strdcpy(buffer2, waiting_hosts[j]->ip);
                        }
                        buffer2[0] = EOS;
                        memcpy(buffer + 1, &nb, sizeof(int));
                        printf("SERVER hostlist (%d) :\n%s \n", nb, buffer + 1);
                        send(poll_set[i + 1].fd, buffer, buffer2 - buffer, 0);
                        removeHost(&number_of_hosts, poll_set, waiting_hosts, i);
                    } else
                        fprintf(stderr, "SERVER : invalid message received.\n");
                }
            }
        }
    } while (TRUE);
}
