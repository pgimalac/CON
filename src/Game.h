#ifndef GAME
#define GAME

#include "Fifo.h"

typedef struct {
	Fifo* moves;
	// le joueur actuel
	// 0 s'il n'y a pas de partie en cours
	int current_player;
	// le plateau
	char board[8][8];
	//Stack moves;
	/*
	0 si la case est vide
	1 si la case est prise par un pion du j1
	2 si la case est prise par un pion du j2
	3 si le joueur courant peut jouer sur la case
	*/

	char block;
} Game;

Game* init();
int play(Game*, int, int);
void reInit(Game*);
void nextPlayer(Game*);
void emptyPossibles(Game*);
int updatePossiblePlay(Game*);
void playTile(Game*, int, int);
void freeGame(Game*);
void cancelMoves(Game*, int);

#endif
