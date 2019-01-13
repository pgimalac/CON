#ifndef STACK_INCLUDE
#define STACK_INCLUDE

#include <stdio.h>

typedef unsigned char Uint8;

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
Uint8 removeFirstFifo(Fifo*);
Uint8 removeLastFifo(Fifo*);
int length(Fifo*);
Uint8 isEmptyFifo(Fifo*);
void freeFifo(Fifo*);
void removeAllFifo(Fifo*);

#endif
