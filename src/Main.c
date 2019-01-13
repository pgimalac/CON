#include <pthread.h>
#include <time.h>
#include <stdio.h>
#include <fcntl.h>

#include "Network.h"
#include "Server.h"
#include "View.h"
#include "Game.h"
#include "Handler.h"

char buffer[FAT_BUFFER_SIZE] = {0};

Game* game;
View* view;
Uint8 keepRunning = TRUE;
int playerSocket;
pthread_t networkT;

/**
 * The arguments and return value aren't used
 * They're here because pthread_create takes a `void *(*start_routine) (void *)`
 */
static void *networkThread(void* p) {
    struct pollfd poll_set = {0};
    poll_set.fd = playerSocket;
    poll_set.revents = POLLIN;

    do {
        printf("%lu : Network Thread begin loop\n", (unsigned long)time(NULL));

        if (poll(&poll_set, 1, -1) == -1) {
            perror("poll");
            keepRunning = FALSE;
            exit(RETURN_ERROR);
        }

        if (poll_set.revents & POLL_ERROR) {
            printPollError(poll_set.revents, "Network thread :");
            keepRunning = FALSE;
            exit(RETURN_ERROR);
        }

        if (poll_set.revents & POLLIN) {
            printf("%lu : Receiving opponent's message ...\n", (unsigned long)time(NULL));
            ssize_t length = recv(poll_set.fd, buffer, FAT_BUFFER_SIZE, MSG_NOSIGNAL);
            if (length == -1) {
                perror("recv");
                keepRunning = FALSE;
                exit(RETURN_ERROR);
            } else if (length == 0) {
                fprintf(stderr, "%lu : opponent left\n", (unsigned long)time(NULL));
                keepRunning = FALSE;
                exit(RETURN_ERROR);
            } else {
                printf("Message received.\n");
                if (buffer[0] == MOVE) {
                    printf("Move received.\n");
                    Uint8 x = buffer[1];
                    Uint8 y = buffer[2];
                    if (play(game, x, y)){
                        printf("Good move.\n");
                        printPieces(view->renderer, game->board, game->current_player);
                    } else
                        fprintf(stderr, "Unexpected message.\n");
                }
            }
        }
    } while (keepRunning);

    return NULL;
}

static void inputThread() {
    print(view);
    printPieces(view->renderer, game->board, game->current_player);

    SDL_Event event;

    printf("<\\ QUIT\n</ MENU\n/>START OVER\n\\>UNDO 1\n");

    while(keepRunning) {
        SDL_WaitEvent(&event);

        if (event.type == SDL_QUIT)
            keepRunning = FALSE;
        if (event.type == SDL_MOUSEBUTTONUP)
            handle_mouse(event.button);
        if (event.type == SDL_KEYDOWN)
            handle_keyboard(event.key);

    }

    freeView(view);
}

static Uint8 playGame(Uint8 type, Uint8 role) {
    Uint8 pos_h = 0;
    if (type == LOCAL_GAME)
        pos_h = MIDDLE_SCREEN;
    else if (type == ONLINE_GAME) {
        if (role == PLAYER1)
            pos_h = LEFT_SCREEN;
        else
            pos_h = RIGHT_SCREEN;
    }

    view = getView(pos_h);
    game = init(type, role);

    inputThread();
    return RETURN_SUCCESS;
}

int main (int argc, char *argv[argc]) {
    int ret = RETURN_ARG_ERROR;

    if (argc == 3 && (strcmp(argv[1], "host") == 0 || strcmp(argv[1], "client") == 0)) {
        if (strlen(argv[2]) < INET_ADDRSTRLEN)
            setServerIpAdress(argv[2]);
        else
            fprintf(stderr, "The second argument is too long to be an ip address.\n");
        argc --;
    }

    if (argc < 2) {
        fprintf(stderr, "Too few arguments !\n");
        return ret;
    }
    if (argc > 2) {
        fprintf(stderr, "Too many arguments !\n");
        return ret;
    }

    do {
        if (strcmp(argv[1], "server") == 0) { // server
            ret = server();
        } else if (strcmp(argv[1], "host") == 0) { // host
            ret = host();
            if (ret > 0) {
                playerSocket = ret;

                if ((networkT = pthread_create(&networkT, NULL, networkThread, NULL)) != 0) {
                    fprintf(stderr, "pthread_create");
                    exit(RETURN_ERROR);
                }
                ret = playGame(ONLINE_GAME, PLAYER1);
            }
        } else if (strcmp(argv[1], "client") == 0) { // client
            ret = client();
            if (ret > 0) {
                playerSocket = ret;

                if ((networkT = pthread_create(&networkT, NULL, networkThread, NULL)) != 0) {
                    fprintf(stderr, "pthread_create");
                    exit(RETURN_ERROR);
                }
                ret = playGame(ONLINE_GAME, PLAYER2);
            }
        } else if (strcmp(argv[1], "local") == 0) { // local game
            ret = playGame(LOCAL_GAME, NONE);
        } else
            fprintf(stderr, "Bad argument\n");
    } while (ret == RETURN_REBOOT);
    return ret;
}
