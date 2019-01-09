#include "Fifo.h"

void addFirstFifo(Fifo* fifo, int x, int y){
    File* file = getFile(x, y);

    file->next = fifo->first;

    if (fifo->first)
        fifo->first->prev = file;
    else
        fifo->last = file;

    fifo->first = file;
}

void addLastFifo(Fifo* fifo, int x, int y){
    File* file = getFile(x, y);

    file->prev = fifo->last;

    if (fifo->last)
        fifo->last->next = file;
    else
        fifo->first = file;

    fifo->last = file;
}

char removeFirstFifo(Fifo* fifo){
    File* file = fifo->first;
    if (file){
        fifo->first = file->next;

        if (file->next)
            (file->next)->prev = NULL;
        else
            fifo->last = NULL;

        free(file);
    }
    return !isEmptyFifo(fifo);
}

char removeLastFifo(Fifo* fifo){
    File* file = fifo->last;
    if (file){
        fifo->last = file->prev;

        if (file->prev != NULL)
            (file->prev)->next = NULL;
        else
            fifo->first = NULL;

        free(file);
    }
    return !isEmptyFifo(fifo);
}

File* getFile(int x, int y){
    File* file = malloc(sizeof(File));

    if (file == NULL){
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    file->x = x;
    file->y = y;
    file->next = NULL;
    file->prev = NULL;

    return file;
}

Fifo* getFifo(){
    Fifo* fifo = malloc(sizeof(Fifo));
    if (fifo == NULL){
        perror("malloc");
        exit(EXIT_FAILURE);
    }


    fifo->first = NULL;
    fifo->last  = NULL;

    return fifo;
}

char isEmptyFifo(Fifo* fifo){
    return fifo == NULL ? -1 : fifo->first == NULL;
}

void freeFifo(Fifo* fifo){
    if (fifo){
        removeAllFifo(fifo);
        free(fifo);
    }
}

void removeAllFifo(Fifo* fifo){
    while (removeLastFifo(fifo)) ;
}
