#include "View.h"
#include "Game.h"

Game* game;
View* view;

int main () {
    int keepPlaying = 1;

    view = getView();
    game = init();

    print(view);
    printPieces(view->renderer, game->board, game->current_player);

    SDL_Event event;

    printf("<\\ QUIT\n</ MENU\n/>START OVER\n\\>UNDO 1\n");

    while(keepPlaying){
        SDL_WaitEvent(&event);
        switch(event.type){
            case SDL_QUIT:
                keepPlaying = 0;
                break;
            case SDL_MOUSEBUTTONUP:
                if (event.button.button == SDL_BUTTON_LEFT){
                    if (game->current_player != 0 && event.motion.x >= LINE_SIZE / 2 && event.motion.x < BOARD_SIZE - LINE_SIZE / 2 &&
                        event.motion.y >= PANEL_SIZE + LINE_SIZE / 2 && event.motion.y < PANEL_SIZE + BOARD_SIZE - LINE_SIZE / 2){
                        if (play(game, event.motion.x / SIZE, (event.motion.y - PANEL_SIZE) / SIZE))
                            printPieces(view->renderer, game->board, game->current_player);
                    }
                    else if (event.motion.x >= START_OVER_BUTTON_X && event.motion.x <= START_OVER_BUTTON_X + START_OVER_BUTTON_W
                        && event.motion.y >= START_OVER_BUTTON_Y && event.motion.y <= START_OVER_BUTTON_Y + START_OVER_BUTTON_H){
                        reInit(game);
                        printPieces(view->renderer, game->board, game->current_player);
                    }
                    else if (event.motion.x >= UNDO_BUTTON_X && event.motion.x <= UNDO_BUTTON_X + UNDO_BUTTON_W
                        && event.motion.y >= UNDO_BUTTON_Y && event.motion.y <= UNDO_BUTTON_Y + UNDO_BUTTON_H){
                        cancelMoves(game, 1);
                        printPieces(view->renderer, game->board, game->current_player);
                    }
                    else if (event.motion.x >= QUIT_BUTTON_X && event.motion.x <= QUIT_BUTTON_X + QUIT_BUTTON_W
                        && event.motion.y >= QUIT_BUTTON_Y && event.motion.y <= QUIT_BUTTON_Y + QUIT_BUTTON_H){
                        keepPlaying = 0;
                    }
                    else if (event.motion.x >= MENU_BUTTON_X && event.motion.x <= MENU_BUTTON_X + MENU_BUTTON_W
                        && event.motion.y >= MENU_BUTTON_Y && event.motion.y <= MENU_BUTTON_Y + MENU_BUTTON_H){
                        printf("Not working yet, probably never. (I may just add buttons in this panel rather than creating a whole menu)\n");
                    }
                }
        }
    }

    freeView(view);
    return EXIT_SUCCESS;
}

