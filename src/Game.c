#include "Game.h"

int play(Game* game, int x, int y){
    if (game->board[x][y] == PLAYABLE){
        playTile(game, x, y);
        nextPlayer(game);
        if (game->current_player != NONE)
            printf("done\n");
        return TRUE;
    }
    return FALSE;
}

void nextPlayer(Game* game){
    if (game->current_player == NONE){
        printf("Error in %s line %d : no current_player player.\n", __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    } else
        game->current_player = OTHER_PLAYER(game->current_player);

    if (updatePossiblePlay(game) == FALSE){
        if (game->block == TRUE) game->current_player = NONE;
        else{
            game->block = TRUE;
            printf("Blocked !\n");
            nextPlayer(game);
        }
    } else game->block = FALSE;
}

Game* init(char gameType, char role){
    Game* game = malloc(sizeof(Game));
    if (!game){
        printf("Error in %s line %d : ", __FILE__, __LINE__);
        perror("");
        exit(EXIT_FAILURE);
    }

    game->moves = getFifo();
    reInit(game, gameType, role);
    return game;
}

void reInit(Game* game, char gameType, char role){
    game->current_player = PLAYER1;
    game->block = FALSE;
    game->gameType = gameType;
    game->role = role;

    removeAllFifo(game->moves);

    for (int i = 0; i < 8; i ++)
        for (int j = 0; j < 8; j ++)
            game->board[i][j] = EMPTY;

    game->board[3][3] = PLAYER1;
    game->board[3][4] = PLAYER2;
    game->board[4][3] = PLAYER2;
    game->board[4][4] = PLAYER1;

    updatePossiblePlay(game);
}

void emptyPossibles(Game* game){
    for (int i = 0; i < 8; i++)
        for (int j = 0; j < 8; j++)
            if (game->board[i][j] == PLAYABLE)
                game->board[i][j] = EMPTY;
}

int updatePossiblePlay(Game* game){
    emptyPossibles(game);
    int num = 0;
    for (int i = 0; i < BOARD_WIDTH; i++){
        for (int j = 0; j < BOARD_WIDTH; j++){
            lbl_inner_loop:
            if (game->board[i][j] != EMPTY) continue; // it is PLAYABLE if we come from the goto
            for (int dx = -1; dx < 2; dx ++){
                for (int dy = -1; dy < 2; dy ++){
                    int x = i;
                    int y = j;
                    if (dx == 0 && dy == 0) continue;
                    char c = -1;
                    do{
                        c ++;
                        x += dx;
                        y += dy;
                    }while(x >= 0 && y >= 0 && x < BOARD_WIDTH && y < BOARD_WIDTH && game->board[x][y] == OTHER_PLAYER(game->current_player));
                    if (c && x >= 0 && y >= 0 && x < BOARD_WIDTH && y < BOARD_WIDTH && game->board[x][y] == game->current_player){
                        game->board[i][j] = PLAYABLE;
                        num ++;
                        j++;
                        if (j < 8)
                            goto lbl_inner_loop;
                    }
                }
            }
        }
    }
    return num;
}

void playTile(Game* game, int x, int y){
    Fifo* file = getFifo();
    addFirstFifo(file, x, y);

    for (int dx = -1; dx < 2; dx ++){
        for (int dy = -1; dy < 2; dy ++){
            int tmpN = 0;
            int xx = x + dx;
            int yy = y + dy;
            if (dx == 0 && dy == 0) continue;
            while(xx >= 0 && yy >= 0 && xx < BOARD_WIDTH && yy < BOARD_WIDTH && game->board[xx][yy] == OTHER_PLAYER(game->current_player)) {
                addLastFifo(file, xx, yy);

                tmpN ++;
                xx += dx;
                yy += dy;
            }
            if (xx < 0 || yy < 0 || xx >= BOARD_WIDTH || yy >= BOARD_WIDTH || game->board[xx][yy] != game->current_player){
                for (int i = 0; i < tmpN; i ++)
                    removeLastFifo(file);
            }
        }
    }

    do
        game->board[file->first->x][file->first->y] = game->current_player;
    while (removeFirstFifo(file));
    addLastFifo(game->moves, x, y);
}

// annule les n derniers coups
void cancelMoves(Game* game, int n){
    Fifo* fifo = game->moves;
    game->moves = getFifo();
    reInit(game, game->gameType, game->role);

    for (int i = 0; i < n && removeLastFifo(fifo); i++) ;

    File* file = (fifo->first);
    while(file){
        playTile(game, file->x, file->y);
        nextPlayer(game);
        file = file->next;
    }

    freeFifo(fifo);
}

void freeGame(Game* game){
    freeFifo(game->moves);
    freeGame(game);
}
