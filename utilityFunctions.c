#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "dataStructures.h"


// FREE DATI
void freeGraph(grafo *g) {
    if (g == NULL) return;

    // Libero l'array degli archi uscenti
    free(g->out);

    // Utilizzo la funzione freeInmap per liberare le strutture all'interno di inmap
    for(int i=0; i<g->N; i++)
        freeInmap(g->in[i], g->N);

    free(g->in);

    free(g);

}


// SALTA COMMENTI
int readFirstNonCommentLine(FILE *file, int *numeroNodi, int *numeroArchi) {
    char line[1024];
    while (fgets(line, sizeof(line), file) != NULL) {
        if (line[0] != '%') {
            // La riga non è un commento, quindi la leggiamo
            sscanf(line, "%d %*d %d", numeroNodi, numeroArchi);
            // Indica che abbiamo letto la prima riga non commento
            return 1;
        }
    }
    return 0; // Non ci sono righe non commento nel file
}


// HEAP PER TOP K NODI

void scambia(NodoHeap *a, NodoHeap *b) {
    NodoHeap temp = *a;
    *a = *b;
    *b = temp;
}

int compare(const void *a, const void *b) {
    double diff = ((NodoHeap*)a)->value - ((NodoHeap*)b)->value;
    if (diff > 0) return 1;
    if (diff < 0) return -1;
    return 0;
}


void heapify(NodoHeap *heap, int n, int i) {
    int smallest = i;
    int left = 2 * i + 1;
    int right = 2 * i + 2;

    if (left < n && heap[left].value < heap[smallest].value)
        smallest = left;

    if (right < n && heap[right].value < heap[smallest].value)
        smallest = right;

    if (smallest != i) {
        scambia(&heap[i], &heap[smallest]);
        heapify(heap, n, smallest);
    }
}

void construisciHeap(NodoHeap *heap, int n) {
    for (int i = n / 2 - 1; i >= 0; i--)
        heapify(heap, n, i);
}

void trovaTopK(double *arr, int size, double *result, int *indices, int k, double *sumOfRanks) {
    // Creao un min heap di dimensione N
    NodoHeap *minHeap = (NodoHeap*)malloc(k * sizeof(NodoHeap));
    for (int i = 0; i < k; i++) {
        minHeap[i].value = arr[i];
        minHeap[i].index = i;
        *sumOfRanks += arr[i];
    }

    // Costruisco il min heap
    construisciHeap(minHeap, k);

    // Iterare sul resto dell'array
    for (int i = k; i < size; i++) {
        *sumOfRanks += arr[i];
        if (arr[i] > minHeap[0].value) {
            minHeap[0].value = arr[i];
            minHeap[0].index = i;
            heapify(minHeap, k, 0);
        }
    }

    // Copio i valori e gli indici nel risultato
    for (int i = 0; i < k; i++) {
        result[i] = minHeap[i].value;
        indices[i] = minHeap[i].index;
    }

    // Ordino il risultato e gli indici
    // Uso una funzione di confronto per ordinare basato sui valori
    qsort(minHeap, k, sizeof(NodoHeap), compare);

    for (int i = 0; i < k; i++) {
        result[i] = minHeap[i].value;
        indices[i] = minHeap[i].index;
    }

    free(minHeap);
}


// termina un processo con eventuale messaggio d'errore
void termina(const char *messaggio)
{
  if(errno==0)  fprintf(stderr,"== %d == %s\n",getpid(), messaggio);
  else fprintf(stderr,"== %d == %s: %s\n",getpid(), messaggio, strerror(errno));
  exit(1);
}

void controlloIdentificatoriNodi(int v1, int v2, int nNodi)
{
    if(v1<0||v1>nNodi)
        termina("Errore: Identificatore di nodo non valido");
    if(v2<0||v2>nNodi)
        termina("Errore: Identificatore di nodo non valido");
}