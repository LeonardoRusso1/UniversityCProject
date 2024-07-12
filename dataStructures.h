#ifndef DATA_STRUCTURES_H
#define DATA_STRUCTURES_H

#include <pthread.h>
#include <semaphore.h>
#include "set.h"

typedef struct {
    int primo;
    int secondo;
} arco;

// HEAP
typedef struct {
    double value;
    int index;
} NodoHeap;

// THREADS
typedef struct {
    grafo *grafo;
    arco *buffer;
    int *cindex;
    int *deadEndNodes;
    int *archiValidi;
    pthread_mutex_t *mutexBuffer;
    pthread_mutex_t *mutexGrafo;
    sem_t *sem_free_slots;
    sem_t *sem_data_items;
} datiCons;

typedef struct {
    grafo *grafo;
    int taux;
    double *x;
    double *y;
    double *xnext;
    double d;
    double *DE;
    double teleport;
    int startBlocco;
    int endBlocco;
    double valorePasso0;
    double *sommatoriaDE;
    int *indicePerCalcoloNext;
    double *errore;
    bool *stop;
    pthread_mutex_t *m;

    int *tf1;
    int *tf2;
    pthread_cond_t *cond_fineFase1;
    pthread_cond_t *cond_fineFase2;
    pthread_mutex_t *mFase;
} datiCalc;


typedef struct {
    int numeroNodi;
    int *numeroIterazione;
    bool *esecuzioneTerminata;
    double *x;
} datiHandler;


#endif // DATA_STRUCTURES_H