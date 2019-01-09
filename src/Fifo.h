#ifndef STACK
#define STACK

#include <stdio.h>
#include <stdlib.h>

typedef struct File File;

// une pile de coordonnées
struct File {
	File *next;
	File *prev;
	int x;
	int y;
};

typedef struct {
	File *first;
	File *last;
} Fifo;

Fifo* getFifo();
File* getFile(int, int);
void addFirstFifo(Fifo*, int, int);
void addLastFifo(Fifo*, int, int);
char removeFirstFifo(Fifo*);
char removeLastFifo(Fifo*);
int length(Fifo*);
char isEmptyFifo(Fifo*);
void freeFifo(Fifo*);
void removeAllFifo(Fifo*);

#endif
