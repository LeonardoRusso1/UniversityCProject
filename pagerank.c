#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <signal.h>
#include "utilityFunctions.h"
#include "pagerank.h"
#include "threads.h"


double *pagerank(grafo *g, double d, double eps, int maxiter, int taux, int *numiter)
{

    double *x = malloc(sizeof(double)*g->N);
    double *y = malloc(sizeof(double)*g->N);
    double *xnext = malloc(sizeof(double)*g->N);

    if (x == NULL || y == NULL || xnext == NULL)
        termina("Errore: Errore durante allocazione memoria dei vettori pagerank");

    // INIZIALIZZAZIONE

    // Termine del pagerank iniziale
    double valorePasso0 = 1.0/g->N;

    // PRIMO TERMINE non dipende da passo o da elemento => 1 volta sola
    double termineTeleporting = (1.0-d)/g->N;

    // S_t
    double DE = 0;
    double primoTermineDE = d/g->N;
    double sommatoriaDE = 0;

    // Y_it 
    int indicePerCalcoloNext = 0;

    // et
    double errore = 1.0;

    pthread_t ids[taux];
    datiCalc datiCalc[taux];

    pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t mFase = PTHREAD_MUTEX_INITIALIZER;

    pthread_cond_t cond_fineFase1;
    pthread_cond_t cond_fineFase2;

    pthread_cond_init(&cond_fineFase1, NULL);
    pthread_cond_init(&cond_fineFase2, NULL);

    // Dati handler
    bool primaEsecuzioneTerminata = false;
    pthread_t idHandler;
    datiHandler datiHandler;
    datiHandler.esecuzioneTerminata = &primaEsecuzioneTerminata;
    datiHandler.numeroNodi = g->N;
    datiHandler.numeroIterazione = numiter;
    datiHandler.x = x;

    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    sigaddset(&mask, SIGUSR2);
    pthread_sigmask(SIG_BLOCK, &mask, NULL);

    pthread_create(&idHandler,NULL,bodyHandler,&datiHandler);

    int n = 0;
    bool stop = false;
    int tf1 = 0;
    int tf2 = 0;
    for(int i=0;i<taux;i++)
    {
        datiCalc[i].x = x;
        datiCalc[i].y = y;
        datiCalc[i].xnext = xnext;
        datiCalc[i].d = d;
        datiCalc[i].DE = &DE;
        datiCalc[i].taux = taux;
        datiCalc[i].grafo = g;
        n = g->N/(taux);
        datiCalc[i].startBlocco = n*i;
        datiCalc[i].endBlocco = (i==taux-1) ? g->N : n*(i+1);
        datiCalc[i].valorePasso0 = valorePasso0;
        datiCalc[i].teleport = termineTeleporting;
        datiCalc[i].sommatoriaDE = &sommatoriaDE;
        datiCalc[i].stop = &stop;
        datiCalc[i].indicePerCalcoloNext = &indicePerCalcoloNext;
        datiCalc[i].errore = &errore;
        datiCalc[i].m = &m;
        datiCalc[i].tf1 = &tf1;
        datiCalc[i].tf2 = &tf2;
        datiCalc[i].mFase = &mFase;
        datiCalc[i].cond_fineFase1 = &cond_fineFase1;
        datiCalc[i].cond_fineFase2 = &cond_fineFase2;
        pthread_create(&ids[i],NULL,bodyCalcolatore,&datiCalc[i]);
    }


    while(true)
    {

        (*numiter)++;

        pthread_mutex_lock(&mFase);

        // Aspetto che gli ausiliari terminino la scansione dei loro blocchi
        while (tf1 < taux)
            pthread_cond_wait(&cond_fineFase1, &mFase);

        tf1 = 0;

        // Calcolo DE per il prossimo calcolo degli ausiliari
        DE = primoTermineDE*sommatoriaDE;

        // Azzero errore siccome sta per cominciare il calcolo degli ausiliari
        errore = 0;

        pthread_cond_broadcast(&cond_fineFase2);

        // Aspetto che gli ausiliari calcolino xnext
        while (tf2 < taux)
            pthread_cond_wait(&cond_fineFase2, &mFase);

        tf2 = 0;

        // 4. x = xnext
        double *temp = x;
        x = xnext;
        xnext = temp;
        for(int i=0;i<taux;i++) {
            datiCalc[i].x = x;
            datiCalc[i].xnext = xnext;
        }

        indicePerCalcoloNext = 0;
        sommatoriaDE = 0;


        if(errore < eps || *numiter >= maxiter){
            stop = true;
            pthread_cond_broadcast(&cond_fineFase1);
            pthread_mutex_unlock(&mFase);
            break;
        }

        pthread_cond_broadcast(&cond_fineFase1);

        // Lascio gli ausiliari continuare
        pthread_mutex_unlock(&mFase);

        // Comando per stampare nodo con rank maggiore ad ogni iterazione
        // pthread_kill(idHandler, SIGUSR1);

    }

    

    // Attendo ausiliari
    for(int i=0;i<taux;i++)
        pthread_join(ids[i],NULL);

    // Kill Handler
    pthread_kill(idHandler, SIGUSR2);
    pthread_join(idHandler, NULL);

    // Pulizia variabili
    pthread_cond_destroy(&cond_fineFase1);
    pthread_cond_destroy(&cond_fineFase2);
    pthread_mutex_destroy(&mFase);

    pthread_mutex_destroy(&m);

    free(y);
    free(xnext);

    return x;
}