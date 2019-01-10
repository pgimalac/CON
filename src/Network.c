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

int getTalkToServerSock(char* server_ip, int port){
    if (server_ip == NULL){
        printf("Enter server ip : \n");
        fgets(server_ip, INET_ADDRSTRLEN, stdin);
    }
    printf("%s\n", server_ip);

    struct sockaddr_in connect_to_server_addr;
    int talkToServerSock;

    // creating the socket used to communicate with the server
    if ((talkToServerSock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket talkToServerSock creation error \n");
        exit(EXIT_FAILURE);
    }

    memset(&connect_to_server_addr, '0', sizeof(connect_to_server_addr));

    connect_to_server_addr.sin_family = AF_INET;
    connect_to_server_addr.sin_port = htons(port);

    if(inet_pton(AF_INET, server_ip, &connect_to_server_addr.sin_addr)<=0) {
        printf("\nInvalid address/ Address not supported \n");
        exit(EXIT_FAILURE);
    }

    if (connect(talkToServerSock, (struct sockaddr *)&connect_to_server_addr, sizeof(connect_to_server_addr)) < 0) {
        printf("\nConnection to the server failed \n");
        exit(EXIT_FAILURE);
    }
    // end

    return talkToServerSock;
}

int getTalkToClientSock(int port){
    int opt = 1;
    int talkToClientSock;
    // creating the local server to talk with the client
    if ((talkToClientSock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket talkToClientSock creation error \n");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(talkToClientSock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    connect_to_client_addr.sin_family = AF_INET;
    connect_to_client_addr.sin_addr.s_addr = INADDR_ANY;
    connect_to_client_addr.sin_port = htons(port);

    if (bind(talkToClientSock, (struct sockaddr*)&connect_to_client_addr, sizeof(connect_to_client_addr))<0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(talkToClientSock, 1) < 0){
        perror("listen");
        exit(EXIT_FAILURE);
    }
    // end
    return talkToClientSock;
}

int clientHost (){
    struct pollfd poll_set[2] = {0};
    poll_set[0].fd = getTalkToServerSock(SERVER_IP_ADDRESS, SERVER_PORT);
    poll_set[0].events = POLLIN;
    poll_set[1].fd = getTalkToClientSock(HOST_PORT);
    poll_set[1].events = POLLIN;

    char name[NAME_LENGTH + 1];
    name[0] = HOST_SERVER_NAME;

    do {
        printf("Enter your nickname (between 2 and %d char) : ", NAME_LENGTH - 1);
        fgets(name + 1, NAME_LENGTH, stdin);
    } while (name[1] == EOS || name[2] == EOS);

    if (send(poll_set[0].fd, name, NAME_LENGTH + 1, 0) <= 0){
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
                printf("HOST_TO_SERVER : SERVER shut down !\n");
                perror("recv");
                close (poll_set[0].fd);
                return RETURN_ERROR;
            }

            printf("HOST_TO_SERVER : received %ld messages\n", length);
            // TODO
        }

        // talk to client part
        if (poll_set[1].revents & POLL_ERROR){
            printf("HOST_TO_CLIENT : error.\n");
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

void printHostList(int size, char buffer[size]) {
    printf("%s\n", buffer);
}

int client () {
    int choice = CLIENT_HOSTLIST_REFRESH;
    char buffer[FAT_BUFFER_SIZE];
    int talkToHostSock;
    do{
        if (choice == CLIENT_HOSTLIST_REFRESH){
            printf("CLIENT : refreshing host list from server\n");
            int talkToServerSock = getTalkToServerSock(SERVER_IP_ADDRESS, SERVER_PORT);

            struct pollfd poll_fd;
            poll_fd.fd = talkToServerSock;
            poll_fd.events = POLLIN;

            printf("CLIENT : sending the refresh hostlist demand.\n");
            char client_server_hostlist = CLIENT_SERVER_HOSTLIST;
            send(talkToServerSock, &client_server_hostlist, sizeof(client_server_hostlist), 0);

            printf("CLIENT : start waiting.\n");
            poll(&poll_fd, 1, 5000);
            if (poll_fd.revents & POLLIN){
                int length = recv(talkToServerSock, buffer, FAT_BUFFER_SIZE, 20);
                printf("CLIENT : Host list (length %d) received from server : \n", length);
                printHostList(length, buffer);
            } else if (poll_fd.revents & POLL_ERROR) {
                fprintf(stderr, "CLIENT : error talking to the server.\n");
                return RETURN_ERROR;
            } else {
                printf("Revents : %d\n", poll_fd.revents);
                close(talkToServerSock);
                continue;
            }

            close(talkToServerSock);
            printf("CLIENT : Enter the number of the host, %d to refresh or any other number to quit.\n", CLIENT_HOSTLIST_REFRESH);
            if (scanf("%d", &choice) < 1 || (choice <= 0 && choice != CLIENT_HOSTLIST_REFRESH))
                choice = CLIENT_QUIT;
            else {
                int k;
                choice = 2 * choice - 1;
                for (k = 1; buffer[k] != EOS && choice > 0; k ++){
                    if (buffer[k] == '\n')
                        choice --;
                }
                for (int j = 0; buffer[k + j] != EOS; j++)
                    if (buffer[k + j] == '\n'){
                        buffer[k + j] = EOS;
                        break;
                    }

                // try to connect to the selected host
                printf("CLIENT : Connecting to the host...\n");
                if ((talkToHostSock = getTalkToServerSock(buffer + k, HOST_PORT)) < 0) {
                    fprintf(stderr, "CLIENT : Error connecting to the host.\n");
                    choice = CLIENT_HOSTLIST_REFRESH;
                } else {
                    printf("Successful connection !\n");
                    return talkToHostSock;
                }
            }
        }
    } while (choice != CLIENT_QUIT);

    return RETURN_SUCCESS;
}

void removeHost(int *number_of_hosts, struct pollfd poll_set[*number_of_hosts + 1], Host* waiting_hosts[*number_of_hosts + 1], int i){
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
    int talkToClientSock = getTalkToClientSock(SERVER_PORT), addrlen = sizeof(connect_to_client_addr), new_socket, valread;

    Host *waiting_hosts[TOTAL_NUMBER_OF_HOSTS];
    struct pollfd poll_set[1 + TOTAL_NUMBER_OF_HOSTS] = {0};
    poll_set[0].fd = talkToClientSock;
    poll_set[0].events = POLLIN;
    int number_of_hosts = 0;

    do{
        printf("SERVER : POLLing.\n");
        poll(poll_set, number_of_hosts + 1, -1);

        if (poll_set[0].revents & POLL_ERROR){
            fprintf(stderr, "SERVER : the receiving socket has a problem ; %s\n", poll_set[0].revents == POLLERR ? "POLLERR" : "POLLHUP");
            for (int i = 0; i < number_of_hosts + 1; i++)
                close(poll_set[i].fd);
            printf("Rebooting...\n");
            return RETURN_REBOOT;
        }

        if (number_of_hosts < TOTAL_NUMBER_OF_HOSTS && poll_set[0].revents & POLLIN){
            printf("SERVER : Adding a new host (now number %d)\n", number_of_hosts);
            if ((new_socket = accept(talkToClientSock, (struct sockaddr *)&connect_to_client_addr, (socklen_t*)&addrlen))<0){
                fprintf(stderr, "SERVER : error adding an host\n");
                continue;
            }
            waiting_hosts[number_of_hosts] = malloc(sizeof(Host));
            waiting_hosts[number_of_hosts]->name[0] = EOS;
            inet_ntop(AF_INET, &connect_to_client_addr.sin_addr, waiting_hosts[number_of_hosts]->ip, INET_ADDRSTRLEN);

            number_of_hosts ++;

            poll_set[number_of_hosts].fd = new_socket;
            poll_set[number_of_hosts].events = POLLIN;
        }

        for (int i = 0; i < number_of_hosts; i++){
            if (poll_set[i + 1].revents & (POLLERR | POLLHUP | POLLNVAL)){
                removeHost(&number_of_hosts, poll_set, waiting_hosts, i);
            } else if (poll_set[i + 1].revents & POLLIN){
                valread = recv(poll_set[i + 1].fd, &buffer, FAT_BUFFER_SIZE, 0);

                if (valread <= 0){
                    removeHost(&number_of_hosts, poll_set, waiting_hosts, i);
                } else {
                    buffer[valread] = EOS;
                    printf("length of the message %d, message : \"%s\"\n", valread, buffer);
                    if (buffer[0] == HOST_SERVER_NAME){ // change the name of the host
                        if (valread > NAME_LENGTH + 1)
                            printf("SERVER : too long name\n");
                        else {
                            printf("SERVER : changing an host's name.\n");
                            for (int k = 0, cursor = 0; *(buffer + k) != EOS; k++){
                                if (*(buffer + 1 + k) != '\n'){
                                    waiting_hosts[i]->name[cursor] = *(buffer + 1 + k);
                                    cursor ++;
                                }
                            }

                            printf("SERVER : the name of %d was changed to \"%s\"\n", poll_set[i + 1].fd, waiting_hosts[i]->name);
                        }
                    } else if (buffer[0] == HOST_SERVER_QUIT){ // the host left
                        printf("SERVER : An host is gone...\n");
                        removeHost(&number_of_hosts, poll_set, waiting_hosts, i);
                    } else if (buffer[0] == CLIENT_SERVER_HOSTLIST){ // a client asks the host list
                        printf("SERVER : a client asked the host list.\n");
                        buffer[0] = SERVER_CLIENT_HOSTLIST;
                        int cursor = 1;
                        for (int j = 0; j < number_of_hosts; j ++){
                            printf("%d : %s ; %s\n", poll_set[j + 1].fd, waiting_hosts[j]->ip, waiting_hosts[j]->name);

                            if (waiting_hosts[j]->name[0] == EOS)
                                continue;

                            for (int k = 0; (buffer[cursor] = waiting_hosts[j]->name[k]) != EOS; k++, cursor ++)
                                ;
                            buffer[cursor] = '\n';
                            cursor ++;
                            for (int k = 0; (buffer[cursor] = waiting_hosts[j]->ip[k]) != EOS; k++, cursor ++)
                                ;
                            buffer[cursor] = '\n';
                            cursor ++;
                        }
                        buffer[cursor] = EOS;
                        printf("SERVER hostlist : %s\n", buffer);
                        send(poll_set[i + 1].fd, buffer, cursor, 0);
                        removeHost(&number_of_hosts, poll_set, waiting_hosts, i);
                    } else
                        printf("SERVER : invalid message received.\n");
                }
            }
        }
    } while (TRUE);
    return RETURN_SUCCESS;
}
