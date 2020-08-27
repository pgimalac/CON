#include "Handler.h"

extern char buffer[FAT_BUFFER_SIZE];
extern Game *game;
extern View *view;
extern int playerSocket;
extern Uint8 keepRunning;
extern Fifo *draw_events;
extern SDL_cond *cond;

static Uint8 button_clicked(int click_x, int click_y, int x, int y, int w,
                            int h) {
    return click_x >= x && click_x < x + w && click_y >= y && click_y < y + h;
}

void handle_mouse(SDL_MouseButtonEvent mouse_event) {
    printf("Mouse button clicked.\n");
    if (mouse_event.button != SDL_BUTTON_LEFT)
        return;

    printf("Left mouse button clicked.\n");
    if ((game->gameType == ONLINE_GAME ? game->current_player == game->role
                                       : game->current_player != NONE) &&
        button_clicked(mouse_event.x, mouse_event.y, LINE_SIZE / 2,
                       PANEL_SIZE + LINE_SIZE / 2, BOARD_SIZE - LINE_SIZE,
                       BOARD_SIZE - LINE_SIZE)) {

        Uint8 x = mouse_event.x / SIZE, y = (mouse_event.y - PANEL_SIZE) / SIZE;
        printf("Clicked on the board panel : (%d, %d).\n", x, y);
        if (play(game, x, y)) {
            printf("%lu : Valid move.\n", (unsigned long)time(NULL));
            if (game->gameType ==
                ONLINE_GAME) { // if it is an online game it means the player is
                               // the good one
                printf("%lu : Online game : sending the move.\n",
                       (unsigned long)time(NULL));

                buffer[0] = CLIENT_CLIENT_MOVE;
                buffer[1] = x;
                buffer[2] = y;
                if (send(playerSocket, buffer, 3, MSG_NOSIGNAL) < 0) {
                    printf("%lu : ", (unsigned long)time(NULL));
                    perror("Network error sending the message ");
                    exit(RETURN_ERROR);
                }
                printf("Message successfully sent.\n");
            }
            addFirstFifo(draw_events, DRAW_TILES, 0);
            SDL_CondSignal(cond);
        }
    } else if (game->gameType == LOCAL_GAME &&
               button_clicked(mouse_event.x, mouse_event.y, START_OVER_BUTTON_X,
                              START_OVER_BUTTON_Y, START_OVER_BUTTON_W,
                              START_OVER_BUTTON_H)) {
        reInit(game, game->gameType, game->role);
        addFirstFifo(draw_events, DRAW_TILES, 0);
        SDL_CondSignal(cond);
    } else if (game->gameType == LOCAL_GAME &&
               button_clicked(mouse_event.x, mouse_event.y, UNDO_BUTTON_X,
                              UNDO_BUTTON_Y, UNDO_BUTTON_W, UNDO_BUTTON_H)) {
        cancelMoves(game, 1);
        addFirstFifo(draw_events, DRAW_TILES, 0);
        SDL_CondSignal(cond);
    } else if (button_clicked(mouse_event.x, mouse_event.y, QUIT_BUTTON_X,
                              QUIT_BUTTON_Y, QUIT_BUTTON_W, QUIT_BUTTON_H)) {
        keepRunning = FALSE;
    } else if (button_clicked(mouse_event.x, mouse_event.y, MENU_BUTTON_X,
                              MENU_BUTTON_Y, MENU_BUTTON_W, MENU_BUTTON_H)) {
        printf(
            "This button is not working yet, probably never will. (I may just "
            "add buttons in this panel rather than creating a whole menu)\n");
    }
}

void handle_keyboard(SDL_KeyboardEvent key_event) {
    printf("key pressed.\n");
    if (key_event.keysym.sym == SDLK_r) {
        printf("Refresh view.\n");
        addFirstFifo(draw_events, DRAW_MENU, 0);
        addFirstFifo(draw_events, DRAW_TILES, 0);
        SDL_CondSignal(cond);
    }
}
