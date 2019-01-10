#include "View.h"

void fillCircles(SDL_Renderer* renderer, int size, int xx[size], int yy[size], int radius){
    for (int i = 0; i < radius; i++){
        for (int j = 0; i >= j && (i * i) + (j * j) <= (radius * radius); j++){
            for (int k = 0; k < size; k ++){
                SDL_RenderDrawPoint(renderer, xx[k] + i, yy[k] + j);
                SDL_RenderDrawPoint(renderer, xx[k] + i, yy[k] - j);
                SDL_RenderDrawPoint(renderer, xx[k] - i, yy[k] + j);
                SDL_RenderDrawPoint(renderer, xx[k] - i, yy[k] - j);
                SDL_RenderDrawPoint(renderer, xx[k] + j, yy[k] + i);
                SDL_RenderDrawPoint(renderer, xx[k] + j, yy[k] - i);
                SDL_RenderDrawPoint(renderer, xx[k] - j, yy[k] + i);
                SDL_RenderDrawPoint(renderer, xx[k] - j, yy[k] - i);
            }
        }
    }
    SDL_RenderPresent(renderer);
}

void printLines(View* view){
    SDL_SetRenderDrawColor(view->renderer, LINE_R, LINE_G, LINE_B, LINE_A);
    (view->line_small_V)->x = 0;
    (view->line_small_V)->y = PANEL_SIZE;
    (view->line_small_H)->x = 0;
    (view->line_small_H)->y = PANEL_SIZE;
    (view->line_V)->y = PANEL_SIZE;
    (view->line_H)->x = 0;

    for (int i = 1; i < 8; i ++){
        // lignes horizontales
        (view->line_H)->y = PANEL_SIZE + i * SIZE - LINE_SIZE / 2;
        SDL_RenderFillRect(view->renderer, view->line_H);

        // lignes verticales
        (view->line_V)->x = i * SIZE - LINE_SIZE / 2;
        SDL_RenderFillRect(view->renderer, view->line_V);
    }

    // top horizontal
    SDL_RenderFillRect(view->renderer, view->line_small_H);

    // left vertical
    SDL_RenderFillRect(view->renderer, view->line_small_V);

    // bot horizontal
    (view->line_small_H)->y = PANEL_SIZE + BOARD_SIZE - LINE_SIZE / 2;
    SDL_RenderFillRect(view->renderer, view->line_small_H);

    // right vertical
    (view->line_small_V)->x = BOARD_SIZE - LINE_SIZE / 2;
    SDL_RenderFillRect(view->renderer, view->line_small_V);
}

void printTiles(View* view){
    SDL_SetRenderDrawColor(view->renderer, TILE_R, TILE_G, TILE_B, TILE_A);

    (view->tile)->x = LINE_SIZE / 2;
    for (int i = 0; i < 8; i ++){
        for (int j = 0; j < 8; j ++){
            (view->tile)->y = PANEL_SIZE + j * SIZE + LINE_SIZE / 2;
            SDL_RenderFillRect(view->renderer, view->tile);
        }
        (view->tile)->x += SIZE;
    }

}

void printMenu(View* view){
    SDL_SetRenderDrawColor(view->renderer, HEADER_PANEL_R, HEADER_PANEL_G, HEADER_PANEL_B, HEADER_PANEL_A);
    SDL_RenderFillRect(view->renderer, view->header_panel);

    SDL_SetRenderDrawColor(view->renderer, QUIT_BUTTON_R, QUIT_BUTTON_G, QUIT_BUTTON_B, QUIT_BUTTON_A);
    SDL_RenderFillRect(view->renderer, view->quitButton);

    SDL_SetRenderDrawColor(view->renderer, START_OVER_BUTTON_R, START_OVER_BUTTON_G, START_OVER_BUTTON_B, START_OVER_BUTTON_A);
    SDL_RenderFillRect(view->renderer, view->startOverButton);

    SDL_SetRenderDrawColor(view->renderer, UNDO_BUTTON_R, UNDO_BUTTON_G, UNDO_BUTTON_B, UNDO_BUTTON_A);
    SDL_RenderFillRect(view->renderer, view->undoButton);

    SDL_SetRenderDrawColor(view->renderer, MENU_BUTTON_R, MENU_BUTTON_G, MENU_BUTTON_B, MENU_BUTTON_A);
    SDL_RenderFillRect(view->renderer, view->menuButton);

    SDL_SetRenderDrawColor(view->renderer, TILE_R, TILE_G, TILE_B, TILE_A);
    SDL_RenderFillRect(view->renderer, view->currentPlayer);
}

void print(View* view){
    SDL_SetRenderDrawColor(view->renderer, 0, 0, 0, 255);
    SDL_RenderClear(view->renderer);

    printMenu(view);
    printLines(view);
    printTiles(view);

    SDL_RenderPresent(view->renderer);
}

View* getView(){
    if (SDL_Init(SDL_INIT_VIDEO)){
        fprintf(stderr, "Erreur au chargement de la librairie graphique. %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    SDL_Window* screen = SDL_CreateWindow("Othello", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          BOARD_SIZE, BOARD_SIZE + PANEL_SIZE, 0);

    if (! screen){
        fprintf(stderr, "Erreur au chargement de la fenêtre. %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(screen, -1, 0);
    View* view = malloc(sizeof(View));

    if (view == NULL){
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    view->screen   = screen;
    view->renderer = renderer;

    view->header_panel    = getSDLRect(0, 0, BOARD_SIZE, PANEL_SIZE);
    view->quitButton      = getSDLRect(QUIT_BUTTON_X, QUIT_BUTTON_Y, QUIT_BUTTON_W, QUIT_BUTTON_H);
    view->startOverButton = getSDLRect(START_OVER_BUTTON_X, START_OVER_BUTTON_Y, START_OVER_BUTTON_W, START_OVER_BUTTON_H);
    view->undoButton      = getSDLRect(UNDO_BUTTON_X, UNDO_BUTTON_Y, UNDO_BUTTON_W, UNDO_BUTTON_H);
    view->menuButton      = getSDLRect(MENU_BUTTON_X, MENU_BUTTON_Y, MENU_BUTTON_W, MENU_BUTTON_H);
    view->currentPlayer   = getSDLRect(CURRENT_PLAYER_X, CURRENT_PLAYER_Y, CURRENT_PLAYER_W, CURRENT_PLAYER_H);

    view->line_small_V = getSDLRect(0, 0, LINE_SIZE / 2, BOARD_SIZE);
    view->line_small_H = getSDLRect(0, 0, BOARD_SIZE, LINE_SIZE / 2);
    view->line_V       = getSDLRect(0, 0, LINE_SIZE, BOARD_SIZE);
    view->line_H       = getSDLRect(0, 0, BOARD_SIZE, LINE_SIZE);
    view->tile         = getSDLRect(0, 0, SIZE - LINE_SIZE, SIZE - LINE_SIZE);

    return view;
}

void freeView(View* view){
    SDL_DestroyWindow(view->screen);
    free(view);
    SDL_Quit();
}

SDL_Rect* getSDLRect(int x, int y, int w, int h){
    SDL_Rect* p = malloc(sizeof(SDL_Rect));
    if (p == NULL){
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    p->x = x;
    p->y = y;
    p->w = w;
    p->h = h;
    return p;
}

void printPieces(SDL_Renderer* renderer, char board[8][8], char player){
    int xx1[64] = {-1};
    int yy1[64] = {-1};
    int xx2[64] = {-1};
    int yy2[64] = {-1};
    int n1d = 0;
    int n1e = 0;
    int n2d = 0;
    int n2e = 0;
    for (int i = 0; i < 8; i ++){
        for (int j = 0; j < 8; j ++){
            char c = board[i][j];
            int xxx = i * SIZE + SIZE / 2;
            int yyy = PANEL_SIZE + j * SIZE + SIZE / 2;
            switch (c){
                case 0 :
                    xx2[n2d] = xxx;
                    yy2[n2d] = yyy;
                    n2d ++;
                    break;
                case 1 :
                    xx1[n1d] = xxx;
                    yy1[n1d] = yyy;
                    n1d ++;
                    break;
                case 2 :
                    xx1[63 - n1e] = xxx;
                    yy1[63 - n1e] = yyy;
                    n1e ++;
                    break;
                case 3 :
                    xx2[63 - n2e] = xxx;
                    yy2[63 - n2e] = yyy;
                    n2e ++;
            }
        }
    }

    if (n1d){
        SDL_SetRenderDrawColor(renderer, PLAYER1_R, PLAYER1_G, PLAYER1_B, PLAYER1_A);
        fillCircles(renderer, n1d, xx1, yy1, RADIUS);
    }
    if (n1e){
        SDL_SetRenderDrawColor(renderer, PLAYER2_R, PLAYER2_G, PLAYER2_B, PLAYER2_A);
        fillCircles(renderer, n1e, xx1 + 64 - n1e, yy1 + 64 - n1e, RADIUS);
    }
    if (n2d){
        SDL_SetRenderDrawColor(renderer, TILE_R, TILE_G, TILE_B, TILE_A);
        fillCircles(renderer, n2d, xx2, yy2, RADIUS);
    }
    if (n2e){
        SDL_SetRenderDrawColor(renderer, POTENTIAL_R, POTENTIAL_G, POTENTIAL_B, POTENTIAL_A);
        fillCircles(renderer, n2e, xx2 + 64 - n2e, yy2 + 64 - n2e, RADIUS);
    }

    if (player == 1)
        SDL_SetRenderDrawColor(renderer, PLAYER1_R, PLAYER1_G, PLAYER1_B, PLAYER1_A);
    else
        SDL_SetRenderDrawColor(renderer, PLAYER2_R, PLAYER2_G, PLAYER2_B, PLAYER2_A);

    int x[1] = {BOARD_SIZE / 2};
    int y[1] = {PANEL_SIZE / 2};
    fillCircles(renderer, 1, x, y, PANEL_SIZE / 2 - BORD);

}
