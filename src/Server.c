#include "Server.h"

char server_ip_address[INET_ADDRSTRLEN] = "127.0.0.1";

void setServerIpAddress(char* add){
    if (add == NULL) return;

    if (strlen(add) >= INET_ADDRSTRLEN)
        fprintf(stderr, "%s is too long to be a valid IPV4 adress.\n", add);
    else
        strcpy(server_ip_address, add);
}

char* getServerIpAddress(){
    return strdup(server_ip_address);
}

int getTalkToClientSock(int port){
    int talkToClientSock = socket(AF_INET, SOCK_STREAM, 0);

    // creating the local server to talk with the client
    if (talkToClientSock < 0) {
        perror("talkToClientSock socket");
        return RETURN_ERROR;
    }

    int opt = 1;
    if (setsockopt(talkToClientSock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) == -1) {
        perror("talkToClientSock setsockopt");
        return RETURN_ERROR;
    }

    connect_to_client_addr.sin_family = AF_INET;
    connect_to_client_addr.sin_addr.s_addr = INADDR_ANY;
    connect_to_client_addr.sin_port = htons(port);

    if (bind(talkToClientSock, (struct sockaddr*)&connect_to_client_addr, sizeof(connect_to_client_addr)) < 0) {
        perror("talkToClientSock bind");
        return RETURN_ERROR;
    }

    if (listen(talkToClientSock, 1) < 0){
        perror("talkToClientSock listen");
        return RETURN_ERROR;
    }
    // end

    return talkToClientSock;
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

int getTalkToServerSock(char* server_ip, int port){
    if (server_ip == NULL || strlen(server_ip) < 7) {
        server_ip = server_ip_address;
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

    if (ret == -1) {
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

void printPollError(int revents, char* st){
    if (revents & POLLNVAL)
        fprintf(stderr, "%s POLLNVAL error with the socket (invalid socket)\n", st);
    if (revents & POLLHUP)
        fprintf(stderr, "%s POLLHUP error with the socket (network error)\n", st);
    if (revents & POLLERR)
        fprintf(stderr, "%s POLLERR error with the socket (network error or player left)\n", st);
}

int server () {
    int talkToClientSock = getTalkToClientSock(SERVER_PORT), new_socket, valread;
    socklen_t addrlen = sizeof(connect_to_client_addr);

    Host *waiting_hosts[TOTAL_NUMBER_OF_HOSTS];
    struct pollfd poll_set[1 + TOTAL_NUMBER_OF_HOSTS] = {0};
    poll_set[0].fd = talkToClientSock;
    poll_set[0].events = POLLIN;
    int number_of_hosts = 0;

    do {
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
            if (waiting_hosts[number_of_hosts] == NULL) {
                perror("calloc");
                for (int i = 0; i < number_of_hosts + 1; i++)
                    close(poll_set[i].fd);
                return RETURN_ERROR;
            }

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

                if (valread < 0){
                    perror("recv");
                    removeHost(&number_of_hosts, poll_set, waiting_hosts, i);
                } else if (valread == 0) {
                    printf("Remove host.\n");
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
                            printf("%d : \"%s\" (%s)\n", poll_set[j + 1].fd, waiting_hosts[j]->name, waiting_hosts[j]->ip);

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
