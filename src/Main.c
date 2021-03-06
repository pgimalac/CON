#include <fcntl.h>
#include <stdio.h>
#include <time.h>

#include "Game.h"
#include "Handler.h"
#include "Network.h"
#include "Server.h"
#include "View.h"

char buffer[FAT_BUFFER_SIZE] = {0};

Game *game;
View *view;
Uint8 keepRunning = TRUE;
int playerSocket;
SDL_Thread *inputT, *networkT;
Fifo *draw_events;
SDL_cond *cond;

/**
 * The arguments and return value aren't used
 * They're here because pthread_create takes a `void *(*start_routine) (void *)`
 */
static int networkThread(void *p) {
    p = p; // to avoid warnings.

    struct pollfd poll_set = {0};
    poll_set.fd = playerSocket;
    poll_set.events = POLLIN;

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
            printf("%lu : Receiving opponent's message ...\n",
                   (unsigned long)time(NULL));
            ssize_t length =
                recv(poll_set.fd, buffer, FAT_BUFFER_SIZE, MSG_NOSIGNAL);
            if (length == -1) {
                perror("recv");
                keepRunning = FALSE;
                exit(RETURN_ERROR);
            } else if (length == 0) {
                fprintf(stderr, "%lu : opponent left\n",
                        (unsigned long)time(NULL));
                keepRunning = FALSE;
                exit(RETURN_ERROR);
            } else {
                printf("Message received.\n");
                if (buffer[0] == CLIENT_CLIENT_MOVE) {
                    printf("Move received.\n");
                    Uint8 x = buffer[1];
                    Uint8 y = buffer[2];
                    if (play(game, x, y)) {
                        printf("Good move.\n");
                        addFirstFifo(draw_events, DRAW_TILES, 0);
                        SDL_CondSignal(cond);
                    } else
                        fprintf(stderr, "Bad move.\n");
                } else
                    fprintf(stderr, "Unexpected message.\n");
            }
        }
    } while (keepRunning);

    return 0;
}

static int inputThread(void *p) {
    p = p; // to avoid warnings.

    SDL_Event event;

    printf("<\\ QUIT\n</ MENU\n/>START OVER\n\\>UNDO 1\n");

    while (keepRunning) {
        if (SDL_WaitEvent(&event) == 0) {
            fprintf(stderr, "Error waiting for an event.\n");
            break;
        }

        if (event.type == SDL_QUIT)
            keepRunning = FALSE;
        if (event.type == SDL_MOUSEBUTTONUP)
            handle_mouse(event.button);
        if (event.type == SDL_KEYDOWN)
            handle_keyboard(event.key);
    }

    freeGame(game);
    freeView(view);

    return 0;
}

static int viewThread(void) {
    SDL_mutex *mut = SDL_CreateMutex();
    cond = SDL_CreateCond();

    draw_events = getFifo();
    print(view);
    printPieces(view->renderer, game->board, game->current_player);

    int x;

    while (keepRunning) {
        printf("viewThread begin\n");

        SDL_mutexP(mut);
        SDL_CondWait(cond, mut);

        while (!isEmptyFifo(draw_events)) {
            printf("draw_events not empty yet.\n");
            x = draw_events->last->x;
            removeLastFifo(draw_events);

            if (x == DRAW_TILES) {
                printf("print pieces\n");
                printPieces(view->renderer, game->board, game->current_player);
            } else if (x == DRAW_MENU)
                print(view);
        }
        printf("viewThread end : no more draw_events\n");
    }

    return 0;
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

    networkT = SDL_CreateThread(networkThread, "Network Thread", NULL);
    if (networkT == NULL) {
        fprintf(stderr, "SDL_CreateThread");
        exit(RETURN_ERROR);
    }

    inputT = SDL_CreateThread(inputThread, "Input Thread", NULL);
    if (inputT == NULL) {
        fprintf(stderr, "SDL_CreateThread");
        exit(RETURN_ERROR);
    }

    viewThread();

    return RETURN_SUCCESS;
}

int main(int argc, char *argv[argc]) {
    int ret = RETURN_ARG_ERROR;

    if (argc == 3 &&
        (strcmp(argv[1], "host") == 0 || strcmp(argv[1], "client") == 0)) {
        if (strlen(argv[2]) < INET_ADDRSTRLEN)
            setServerIpAddress(argv[2]);
        else
            fprintf(stderr,
                    "The second argument is too long to be an ip address.\n");
        argc--;
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
                ret = playGame(ONLINE_GAME, PLAYER1);
            }
        } else if (strcmp(argv[1], "client") == 0) { // client
            ret = client();
            if (ret > 0) {
                playerSocket = ret;
                ret = playGame(ONLINE_GAME, PLAYER2);
            }
        } else if (strcmp(argv[1], "local") == 0) { // local game
            ret = playGame(LOCAL_GAME, NONE);
        } else
            fprintf(stderr, "Bad argument\n");
    } while (ret == RETURN_REBOOT);
    return ret;
}
