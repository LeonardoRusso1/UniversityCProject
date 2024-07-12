#ifndef UTILITY_FUNCTIONS_H
#define UTILITY_FUNCTIONS_H

#include "dataStructures.h"

int readFirstNonCommentLine(FILE *file, int *numeroNodi, int *numeroArchi);

void freeGraph(grafo *g);

void scambia(NodoHeap *a, NodoHeap *b);

int compare(const void *a, const void *b);

void heapify(NodoHeap *heap, int n, int i);

void construisciHeap(NodoHeap *heap, int n);

void trovaTopK(double *arr, int size, double *result, int *indices, int k, double *sumOfRanks);

void termina(const char *messaggio);

void controlloIdentificatoriNodi(int v1, int v2, int nNodi);

#endif // UTILITY_FUNCTIONS_H