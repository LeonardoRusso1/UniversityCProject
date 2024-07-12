#ifndef SET_H
#define SET_H

#include <stdbool.h>

typedef struct {
    int size;
    int capacity;
    int *data; // array di array 
} inmap;

typedef struct {
    int N; // numero dei nodi del grafo
    int *out; // array con il numero di archi uscenti da ogni nodo
    inmap **in; // array con gli insiemi di archi entranti in ogni nodo
} grafo;

grafo *creaGrafo(int numeroNodi);

inmap* createInmap();

void addToInmap(inmap *in, int primo, int secondo, bool *valoreNonEraPresente, int nNodi);

void freeInmap(inmap *map, int numeroNodi);

int binarySearch(int *arr, int left, int right, int value);


#endif // SET_H