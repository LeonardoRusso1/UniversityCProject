#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <getopt.h>
#include "dataStructures.h"
#include "utilityFunctions.h"
#include "threads.h"
#include "pagerank.h"

#define Buf_size 128

int main(int argc, char *argv[])
{
    int k = 3;
    int m = 100;
    double d = 0.9;
    double e = 1e-7;
    int t = 3;

    int opzione;
    while ((opzione = getopt(argc, argv, "k:m:d:e:t:")) != -1)
    {
        switch (opzione) 
        {
            case 'k':
                k = atoi(optarg);
                break;
            case 'm':
                m = atoi(optarg);
                break;
            case 'd':
                d = atof(optarg);
                break;
            case 'e':
                e = atof(optarg);
                break;
            case 't':
                t = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Usage: %s [-k K] [-m M] [-d D] [-e E] [-t T] infile\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    // Verifica che sia stato passato il file di input
    if (optind >= argc) 
        termina("Errore: File di input non inserito");

    // Verifica che il formato del file di input sia '.mtx'
    const char *file_extension = strrchr(argv[optind], '.');
    if (file_extension == NULL || strcmp(file_extension, ".mtx") != 0)
        termina("Errore: Formato del file di input non valido");

    // Apro file di input
    FILE *f = fopen(argv[optind],"r");
    if (f == NULL)
        termina("Errore: Nome file di input non valido");



    // Buffer per lettura file
    arco buffer[Buf_size];

    int prodIndex = 0;
    int consIndex = 0;

    pthread_mutex_t mutexBuffer = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t mutexGrafo = PTHREAD_MUTEX_INITIALIZER;
    sem_t sem_free_slots, sem_data_items;
    sem_init(&sem_free_slots,0,Buf_size);
    sem_init(&sem_data_items,0,0);
    
    // Leggo prima riga del file .mtx saltando i commenti
    int numeroNodi,numeroArchi = 0;

    // Leggo numero Nodi e Archi
    readFirstNonCommentLine(f,&numeroNodi,&numeroArchi);
    int deadEndNodes = numeroNodi;
    int archiValidi = 0;

    // CREAZIONE GRAFO
    grafo *g = creaGrafo(numeroNodi);

    // Creo e avvio thread
    pthread_t ids[t];
    datiCons datiCons[t];
    for(int i=0;i<t;i++)
    {
        datiCons[i].buffer = buffer;
        datiCons[i].cindex = &consIndex;
        datiCons[i].deadEndNodes = &deadEndNodes;
        datiCons[i].archiValidi = &archiValidi;
        datiCons[i].mutexBuffer = &mutexBuffer;
        datiCons[i].mutexGrafo = &mutexGrafo;
        datiCons[i].sem_data_items = &sem_data_items;
        datiCons[i].sem_free_slots = &sem_free_slots;
        datiCons[i].grafo = g;
        pthread_create(&ids[i],NULL,bodyLettore,&datiCons[i]);
    }

    // Leggo archi da file e li scrivo in buffer
    int primo, secondo = 0;
    while(true)
    {

        // Leggo arco
        int e = fscanf(f,"%d %d",&primo,&secondo);

        if(e==EOF)
            break;

        sem_wait(&sem_free_slots);
        // Inserisco arco in buffer
        buffer[prodIndex++ % Buf_size] = (arco){primo, secondo};
        sem_post(&sem_data_items);

    }

    // Chiudo file
    if(fclose(f)!=0)
        termina("Errore: Errore chiusura file di input");;

    // Condizione di terminazione per consumatori
    for(int i=0;i<t;i++)
    {
        sem_wait(&sem_free_slots);
        buffer[prodIndex++ % Buf_size] = (arco){-1, -1};
        sem_post(&sem_data_items);
    }

    for(int i=0;i<t;i++)
        pthread_join(ids[i],NULL);

    int numeroIterazioni = 0;
    double *risultato = pagerank(g,d,e,m,t,&numeroIterazioni);
    
    // Calcolo Top K Nodi
    double sumOfRanks = 0.0;
    double *topKNodes = malloc(k*sizeof(double));
    int *indici = malloc(k*sizeof(int));
    trovaTopK(risultato,g->N,topKNodes,indici,k,&sumOfRanks);

    // Stampo risultati
    fprintf(stdout,"Number of nodes: %d\n",numeroNodi);
    fprintf(stdout,"Number of dead-end nodes: %d\n",deadEndNodes);
    fprintf(stdout,"Number of valid arcs: %d\n",archiValidi);
    if(numeroIterazioni < m)
        fprintf(stdout,"Converged after %d iterations\n",numeroIterazioni);
    else
        fprintf(stdout,"Did not converge after %d iterations\n",numeroIterazioni);
    fprintf(stdout,"Sum of ranks: %.4f   (should be 1)\n",sumOfRanks);
    fprintf(stdout,"Top %d nodes:\n",k);
    for (int i = k-1; i >= 0; i--)
        fprintf(stdout,"  %d %f\n", indici[i], topKNodes[i]);

    // Free risorse
    freeGraph(g);
    free(topKNodes);
    free(indici);
    free(risultato);

    return 0;
}