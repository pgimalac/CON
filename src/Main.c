#ifndef MAIN_INCLUDE
#define MAIN_INCLUDE

#include <pthread.h>
#include <time.h>

#include "Network.h"
#include "View.h"
#include "Game.h"

char buffer[FAT_BUFFER_SIZE];

Game* game;
View* view;
int keepRunning = TRUE;
int playerSocket;
pthread_t networkT;

void *networkThread(void* p){
    struct pollfd poll_set;
    poll_set.fd = playerSocket;
    poll_set.events = POLLIN;
    static int number = 15;
    int ret;
    do {
        printf("%lu : Network Thread begin loop\n", (unsigned long)time(NULL));
        ret = poll(&poll_set, 1, -1) < 0;
        if (ret == -1){
            perror("poll");
        } else if (ret > 0) {
            if (poll_set.revents & (POLLERR | POLLHUP | POLLNVAL)){
                if (poll_set.revents & POLLNVAL)
                    fprintf(stderr, "%lu : POLLNVAL error with the socket (invalid socket)", (unsigned long)time(NULL));
                if (poll_set.revents & POLLHUP)
                    fprintf(stderr, "%lu : POLLHUP error with the socket (network error)", (unsigned long)time(NULL));
                if (poll_set.revents & POLLERR)
                    fprintf(stderr, "%lu : POLLERR error with the socket (network error or player left)", (unsigned long)time(NULL));
                keepRunning = FALSE;
                close(playerSocket);
                exit(RETURN_ERROR);
            }
            if (poll_set.revents & POLLIN){
                printf("%lu : Receiving opponent's message ...\n", (unsigned long)time(NULL));
                ssize_t length = recv(poll_set.fd, buffer, FAT_BUFFER_SIZE, 0);
                if (length == -1){
                    perror("recv");
                    keepRunning = FALSE;
                    close(playerSocket);
                    exit(RETURN_ERROR);
                } else if (length == 0) {
                    printf("%lu : Empty message received.\n", (unsigned long)time(NULL));
                } else {
                    if (buffer[0] == MOVE){
                        char x = buffer[1];
                        char y = buffer[2];
                        if (play(game, x, y))
                            printPieces(view->renderer, game->board, game->current_player);
                    }
                }
/*                printf("Sending ack\n");
                if (send(playerSocket, &ACK, 1, 0) <= 0){
                    perror("Error sending ack ");
                    exit(RETURN_ERROR);
                }
*/          }
        }
    } while (keepRunning && number--);

    return NULL;
}

void *inputThread(void* p){
    print(view);
    printPieces(view->renderer, game->board, game->current_player);

    SDL_Event event;

    printf("<\\ QUIT\n</ MENU\n/>START OVER\n\\>UNDO 1\n");

    while(keepRunning){
        SDL_WaitEvent(&event);
        switch(event.type){
            case SDL_QUIT:
                keepRunning = FALSE;
                break;
            case SDL_MOUSEBUTTONUP:
                printf("Mouse button clicked.\n");
                if (event.button.button == SDL_BUTTON_LEFT){
                    printf("Left mouse button clicked.\n");
                    if ((game->gameType == ONLINE_GAME ? game->current_player == game->role : game->current_player != NONE) && event.motion.x >= LINE_SIZE / 2 && event.motion.x < BOARD_SIZE - LINE_SIZE / 2 &&
                        event.motion.y >= PANEL_SIZE + LINE_SIZE / 2 && event.motion.y < PANEL_SIZE + BOARD_SIZE - LINE_SIZE / 2){

                        char x = event.motion.x / SIZE, y = (event.motion.y - PANEL_SIZE) / SIZE;
                        printf("Clicked on the board panel : (%d, %d).\n", x, y);
                        if (play(game, x, y)){
                            printf("%lu : Valid move.\n", (unsigned long)time(NULL));
                            if (game->gameType == ONLINE_GAME){ // if it is an online game it means the player is the good one
                                printf("%lu : Online game : sending the move.\n", (unsigned long)time(NULL));

                                buffer[0] = MOVE;
                                buffer[1] = x;
                                buffer[2] = y;
                                buffer[3] = EOS;
/*                                char boo = 0;
                                do {
                                    if (boo == 5){
                                        printf("Still no ACK received : connection error\n");
                                        exit(RETURN_ERROR);
                                    }
                                    boo ++;
                                    printf("Sending the move.\n");
*/                                    if (send(playerSocket, buffer, 4, 0) <= 0){
                                        printf("%lu : ", (unsigned long)time(NULL));
                                        perror("Network error sending the message ");
                                        exit(RETURN_ERROR);
                                    }
/*                                    printf("Message sent %d time\n", boo);
                                    if (poll(poll_set, 1, 1) == -1){
                                        perror("Poll error waiting for the ACK ");
                                        exit(RETURN_ERROR);
                                    } else if (poll_set->revents & (POLLNVAL | POLLERR | POLLHUP)){
                                        perror("Socket error waiting for the ACK ");
                                        exit(RETURN_ERROR);
                                    } else if (poll_set->revents & POLLIN){
                                        if (recv(poll_set->fd, buffer, FAT_BUFFER_SIZE, 0) == -1){
                                            perror("Error (maybe) receiving the ACK ");
                                            exit(RETURN_ERROR);
                                        } else {
                                            if (buffer[0] == ACK)
                                                boo = 0;
                                            else
                                                printf("No ACK received.\n");
                                        }
                                    } else
                                        printf("No ACK received.\n");
                                } while(boo != 0);
*/                                printf("Message successfully sent.\n");
                            }
                            printPieces(view->renderer, game->board, game->current_player);
                        }
                    }
                    else if (game->gameType == LOCAL_GAME && event.motion.x >= START_OVER_BUTTON_X && event.motion.x <= START_OVER_BUTTON_X + START_OVER_BUTTON_W
                        && event.motion.y >= START_OVER_BUTTON_Y && event.motion.y <= START_OVER_BUTTON_Y + START_OVER_BUTTON_H){
                        reInit(game, game->gameType, game->role);
                        printPieces(view->renderer, game->board, game->current_player);
                    }
                    else if (game->gameType == LOCAL_GAME && event.motion.x >= UNDO_BUTTON_X && event.motion.x <= UNDO_BUTTON_X + UNDO_BUTTON_W
                        && event.motion.y >= UNDO_BUTTON_Y && event.motion.y <= UNDO_BUTTON_Y + UNDO_BUTTON_H){
                        cancelMoves(game, 1);
                        printPieces(view->renderer, game->board, game->current_player);
                    }
                    else if (event.motion.x >= QUIT_BUTTON_X && event.motion.x <= QUIT_BUTTON_X + QUIT_BUTTON_W
                        && event.motion.y >= QUIT_BUTTON_Y && event.motion.y <= QUIT_BUTTON_Y + QUIT_BUTTON_H){
                        keepRunning = FALSE;
                    }
                    else if (event.motion.x >= MENU_BUTTON_X && event.motion.x <= MENU_BUTTON_X + MENU_BUTTON_W
                        && event.motion.y >= MENU_BUTTON_Y && event.motion.y <= MENU_BUTTON_Y + MENU_BUTTON_H){
                        printf("This button is not working yet, probably never will. (I may just add buttons in this panel rather than creating a whole menu)\n");
                    }
                }
        }
    }

    freeView(view);

    return NULL;
}

int playGame(char type, char role){
    view = getView();
    game = init(type, role);

    inputThread(NULL);
    return RETURN_SUCCESS;
}

int main (int argc, char* argv[argc]) {
    int ret = RETURN_ARG_ERROR;

    if (argc == 3 && (argv[1][0] == 'h' || argv[1][0] == 'c')){
        if (strlen(argv[2]) < INET_ADDRSTRLEN)
            setServerIpAdress(argv[2]);
        else
            printf("The second argument is too long.\n");
        argc --;
    }

    if (argc < 2){
        printf("Too few arguments !\n");
        return ret;
    }
    else if (argc > 2){
        printf("Too many arguments !\n");
        return ret;
    }

    do {
        if (strcmp(argv[1], "server") == 0) { // server
            ret = server();
        } else if (strcmp(argv[1], "host") == 0) { // host
            ret = clientHost();
            if (ret > 0){
                playerSocket = ret;
                if ((networkT = pthread_create(&networkT, NULL, networkThread, NULL)) != 0){
                    perror("Error creating the network thread ");
                    exit(RETURN_ERROR);
                }
                ret = playGame(ONLINE_GAME, PLAYER1);
            }
        } else if (strcmp(argv[1], "client") == 0) { // client
            ret = client();
            if (ret > 0){
                playerSocket = ret;
                if ((networkT = pthread_create(&networkT, NULL, networkThread, NULL)) != 0){
                    perror("Error creating the network thread ");
                    exit(RETURN_ERROR);
                }
                ret = playGame(ONLINE_GAME, PLAYER2);
            }
        } else if (strcmp(argv[1], "local") == 0) { // local game
            ret = playGame(LOCAL_GAME, NONE);
        } else
            printf("Bad argument\n");
    } while (ret == RETURN_REBOOT);
    return ret;
}

#endif
