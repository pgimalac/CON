#ifndef STACK_INCLUDE
#define STACK_INCLUDE

#include <stdio.h>
#include <stdlib.h>

// une pile de coordonnées
typedef struct File {
	struct File *next;
	struct File *prev;
	int x;
	int y;
} File;

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
