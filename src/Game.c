#include "Game.h"

int play(Game* game, int x, int y){
    if (game->board[x][y] == 3){
        playTile(game, x, y);
        nextPlayer(game);
        if (! game->current_player)
            printf("done\n");
        return 1;
    }
    return 0;
}

void nextPlayer(Game* game){
    game->current_player = 3 - game->current_player;
    if (! updatePossiblePlay(game)){
        if (game->block) game->current_player = 0;
        else{
            game->block = 1;
            printf("Blocked !\n");
            nextPlayer(game);
        }
    } else game->block = 0;
}

Game* init(){
    Game* game = malloc(sizeof(Game));
    if (!game){
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    game->moves = getFifo();
    reInit(game);
    return game;
}

void reInit(Game* game){
    game->current_player = 1;
    game->block = 0;

    removeAllFifo(game->moves);

    for (int i = 0; i < 8; i ++)
        for (int j = 0; j < 8; j ++)
            game->board[i][j] = 0;

    game->board[3][3] = 1;
    game->board[3][4] = 2;
    game->board[4][3] = 2;
    game->board[4][4] = 1;

    updatePossiblePlay(game);
}

void emptyPossibles(Game* game){
    for (int i = 0; i < 8; i++)
        for (int j = 0; j < 8; j++)
            if (game->board[i][j] == 3)
                game->board[i][j] = 0;
}

int updatePossiblePlay(Game* game){
    emptyPossibles(game);
    int num = 0;
    for (int i = 0; i < 8; i++){
        for (int j = 0; j < 8; j++){
            lbl_inner_loop:
            if (game->board[i][j]) continue;
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
                    }while(x >= 0 && y >= 0 && x < 8 && y < 8 && game->board[x][y] == 3 - game->current_player);
                    if (c && x >= 0 && y >= 0 && x < 8 && y < 8 && game->board[x][y] == game->current_player){
                        game->board[i][j] = 3;
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
            while(xx >= 0 && yy >= 0 && xx < 8 && yy < 8 && game->board[xx][yy] == 3 - game->current_player) {
                addLastFifo(file, xx, yy);

                tmpN ++;
                xx += dx;
                yy += dy;
            }
            if (xx < 0 || yy < 0 || xx >= 8 || yy >= 8 || game->board[xx][yy] != game->current_player){
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
    reInit(game);

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
