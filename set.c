#include <stdio.h>
#include <stdlib.h>
#include "dataStructures.h"
#include "utilityFunctions.h"

#define INITIAL_ARRAY_SIZE 1

grafo *creaGrafo(int numeroNodi)
{
    grafo *g = (grafo*) malloc(sizeof(grafo));

    g->N = numeroNodi;
    
    // Allocazione dell'array per il numero di archi uscenti da ogni nodo
    g->out = (int *)calloc(numeroNodi, sizeof(int));
    if (g->out == NULL) {
        termina("Errore: Allocazione della memoria per 'out' fallita");
    }

    // Allocazione e inizializzazione della mappa di insiemi per gli archi entranti in ogni nodo
    g->in = (inmap**)malloc(numeroNodi*sizeof(inmap*));
    if (g->in == NULL) {
        free(g->out);
        termina("Errore: Allocazione della memoria per 'inmap' fallita");
    }

    for (int i = 0; i < numeroNodi; i++)
    {
        g->in[i] = createInmap();
        g->out[i] = 0;
    }

    return g;
}

inmap* createInmap()
{
    inmap *in = (inmap *)malloc(sizeof(inmap));
    if(in == NULL)
        termina("Errore: Allocazione della memoria per 'inmap' fallita");

    in->data = (int*) malloc(sizeof(int));
    if(in->data == NULL)
    {
        free(in);
        termina("Errore: Allocazione della memoria per 'inmap data' fallita");
    }

    in->size = 0;

    in->capacity = INITIAL_ARRAY_SIZE;

    return in;
}

void addToInmap(inmap *in, int primo, int secondo, bool *valoreNonEraPresente, int nNodi)
{
    controlloIdentificatoriNodi(primo,secondo,nNodi);

    if(in->size == in->capacity)
    {
        in->capacity *= 2;
        in->data = (int*) realloc(in->data, in->capacity*sizeof(int));
    }

    // Resituisce -1 se già presente, l'indice dove inserirlo altrimenti
    int index = binarySearch(in->data, 0, in->size - 1, primo);

    if(index != -1)
    {
        *valoreNonEraPresente = true;

        // Sposto elementi per fare spazio al nuovo valore
        for (int i = in->size; i > index; i--)
            in->data[i] = in->data[i - 1];

        in->data[index] = primo;
        in->size++;
    }
}


void freeInmap(inmap *map, int numeroNodi)
{
    if(map == NULL)
        return;


    free(map->data);
    free(map);
}

int binarySearch(int *arr, int left, int right, int value)
{
    while (left <= right)
    {
        int mid = left + (right - left) / 2;

        // Se l'elemento è già presente, restituisce l'indice
        if (arr[mid] == value)
            return -1;

        // Se l'elemento è maggiore, cerca nella metà sinistra dell'array
        if (arr[mid] > value)
            right = mid - 1;
        // Altrimenti, cerca nella metà destra dell'array
        else
            left = mid + 1;
    }

    // Se l'elemento non è presente, restituisce l'indice dove dovrebbe essere inserito
    return left;
}
