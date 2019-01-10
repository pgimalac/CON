#ifndef GAME_INCLUDE
#define GAME_INCLUDE

#include "Fifo.h"

/**
 * Board informations
 */
#define BOARD_WIDTH 8

/**
 * Just for more readability
 */
#define TRUE 1
#define FALSE 0

/**
 * Game type
 */
#define NONE 0
#define LOCAL_GAME 1
#define ONLINE_GAME 2

/**
 * Player
 */
#define EMPTY 0
#define PLAYER1 1
#define PLAYER2 2
#define PLAYABLE 3
#define OTHER_PLAYER(player) (3 - player)

/**
 * All informations about the game
 */
typedef struct {
    /**
     * the stack of moves (to be able to cancel)
     */
	Fifo* moves;

    /**
     * the current player
     */
	int current_player;

    /**
     * the board
     */
	char board[BOARD_WIDTH][BOARD_WIDTH];

    /**
     * true if a player was blocked the previous turn, false otherwise
     */
    char block;

    /**
     * NONE, ONLINE_GAME, LOCAL_GAME(, AI_GAME maybe to come)
     */
    char gameType;

    /**
     * used when playing online to know which player is which
     */
    char role;
} Game;

Game* init(char, char);
int play(Game*, int, int);
void reInit(Game*, char, char);
void nextPlayer(Game*);
void emptyPossibles(Game*);
int updatePossiblePlay(Game*);
void playTile(Game*, int, int);
void freeGame(Game*);
void cancelMoves(Game*, int);

#endif
