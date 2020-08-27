#ifndef VIEW_INCLUDE
#define VIEW_INCLUDE

#include <SDL2/SDL.h>
#include <stdlib.h>

#define SIZE 80
#define BOARD_SIZE (8 * SIZE)
#define PANEL_SIZE 160
#define LINE_SIZE 2
#define RADIUS (2 * SIZE / 5)
#define BORD 5
#define BUTTON_SIZE ((PANEL_SIZE - 4 * BORD) / 2)

#define TILE_R 42
#define TILE_G 150
#define TILE_B 0
#define TILE_A 255

#define LINE_R 0
#define LINE_G 0
#define LINE_B 0
#define LINE_A 255

#define HEADER_PANEL_R 255
#define HEADER_PANEL_G 255
#define HEADER_PANEL_B 255
#define HEADER_PANEL_A 255

#define START_OVER_BUTTON_R 0
#define START_OVER_BUTTON_G 255
#define START_OVER_BUTTON_B 0
#define START_OVER_BUTTON_A 255

#define MENU_BUTTON_R 0
#define MENU_BUTTON_G 255
#define MENU_BUTTON_B 255
#define MENU_BUTTON_A 255

#define QUIT_BUTTON_R 255
#define QUIT_BUTTON_G 0
#define QUIT_BUTTON_B 0
#define QUIT_BUTTON_A 255

#define UNDO_BUTTON_R 0
#define UNDO_BUTTON_G 0
#define UNDO_BUTTON_B 255
#define UNDO_BUTTON_A 255

#define PLAYER1_R 0
#define PLAYER1_G 0
#define PLAYER1_B 0
#define PLAYER1_A 255

#define PLAYER2_R 255
#define PLAYER2_G 255
#define PLAYER2_B 255
#define PLAYER2_A 255

#define POTENTIAL_R 150
#define POTENTIAL_G 150
#define POTENTIAL_B 150
#define POTENTIAL_A 128

#define QUIT_BUTTON_X BORD
#define QUIT_BUTTON_Y BORD
#define QUIT_BUTTON_W BUTTON_SIZE
#define QUIT_BUTTON_H BUTTON_SIZE

#define START_OVER_BUTTON_X BOARD_SIZE - BORD - BUTTON_SIZE
#define START_OVER_BUTTON_Y BORD
#define START_OVER_BUTTON_W BUTTON_SIZE
#define START_OVER_BUTTON_H BUTTON_SIZE

#define UNDO_BUTTON_X BOARD_SIZE - BORD - BUTTON_SIZE
#define UNDO_BUTTON_Y PANEL_SIZE - BORD - BUTTON_SIZE
#define UNDO_BUTTON_W BUTTON_SIZE
#define UNDO_BUTTON_H BUTTON_SIZE

#define MENU_BUTTON_X BORD
#define MENU_BUTTON_Y PANEL_SIZE - BORD - BUTTON_SIZE
#define MENU_BUTTON_W BUTTON_SIZE
#define MENU_BUTTON_H BUTTON_SIZE

#define CURRENT_PLAYER_X (BOARD_SIZE - PANEL_SIZE) / 2 + BORD
#define CURRENT_PLAYER_Y BORD
#define CURRENT_PLAYER_W PANEL_SIZE - 2 * BORD
#define CURRENT_PLAYER_H PANEL_SIZE - 2 * BORD

#define LEFT_SCREEN 1
#define MIDDLE_SCREEN 2
#define RIGHT_SCREEN 3

typedef struct {
    SDL_Window *screen;
    SDL_Renderer *renderer;

    SDL_Rect *header_panel;
    SDL_Rect *quitButton;
    SDL_Rect *startOverButton;
    SDL_Rect *undoButton;
    SDL_Rect *menuButton;
    SDL_Rect *currentPlayer;

    SDL_Rect *line_H;
    SDL_Rect *line_V;
    SDL_Rect *line_small_H;
    SDL_Rect *line_small_V;
    SDL_Rect *tile;
} View;

View *getView(Uint8);
void print(View *);
void printPieces(SDL_Renderer *, Uint8[8][8], Uint8);
void printLines(View *);
void printTiles(View *);
void freeView(View *);

#endif
