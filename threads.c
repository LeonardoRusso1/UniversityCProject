#include <stdio.h>
#include <math.h>
#include <signal.h>
#include <unistd.h>
#include "dataStructures.h"

#define Buf_size 128

void *bodyLettore(void *arg)
{
    datiCons *d = (datiCons *)arg;
    arco n;

    while(true)
    {
        sem_wait(d->sem_data_items);

        // Fase 1
        pthread_mutex_lock(d->mutexBuffer);
        n = d->buffer[*(d->cindex) % Buf_size];
        *(d->cindex) += 1;
        pthread_mutex_unlock(d->mutexBuffer);


        // Condizione di terminazione
        if (n.primo == -1 || n.secondo == -1)
        {
            sem_post(d->sem_free_slots);
            break;
        }

        // Fase 2
        if(n.primo != n.secondo)
        {
            bool nonEraPresente = false;

            inmap *in = d->grafo->in[n.secondo-1];

            pthread_mutex_lock(d->mutexGrafo);
            addToInmap(in, n.primo, n.secondo, &nonEraPresente, d->grafo->N);
            if(nonEraPresente)
            {
                if(d->grafo->out[n.primo-1] == 0)
                    *(d->deadEndNodes)-=1;
                d->grafo->out[n.primo-1]++;
                *(d->archiValidi)+=1;
            }
            pthread_mutex_unlock(d->mutexGrafo);

        }

        sem_post(d->sem_free_slots);
    }

    return (void *)NULL;
}



void *bodyCalcolatore(void *arg)
{
    datiCalc *d = (datiCalc *)arg;
    pthread_mutex_t *m = d->m;
    
    // INIZIALIZZAZIONE
    for(int i=d->startBlocco;i<d->endBlocco;i++)
        d->x[i] = d->valorePasso0;

    while(true)
    {

        if(*(d->stop))
            break;

        // CALCOLO DEAD END
        double sommatoriaTemporaneaDE = 0;
        for(int i=d->startBlocco;i<d->endBlocco;i++){
            if(d->grafo->out[i] == 0)
                sommatoriaTemporaneaDE += d->x[i];
            else
                d->y[i] = d->x[i]/d->grafo->out[i];
        }
        pthread_mutex_lock(m);
        *(d->sommatoriaDE) += sommatoriaTemporaneaDE;
        pthread_mutex_unlock(m);


        // Avverto il main che ho terminato di scansionare il blocco
        pthread_mutex_lock(d->mFase);
        *(d->tf1) += 1;
        if(*(d->tf1) == d->taux)
            pthread_cond_signal(d->cond_fineFase1);

        // Aspetto che main esca dal wait e abbia calcolato DE
        pthread_cond_wait(d->cond_fineFase2,d->mFase);
        pthread_mutex_unlock(d->mFase);


        // CALCOLO ELEMENTI VETTORE XNEXT
        double sommatoriaTemporaneaErrore = 0;
        while(true)
        {

            int indiceDaCalcolare = 0;

            // Acquisisco indice elemento da calcolare
            pthread_mutex_lock(m);
            indiceDaCalcolare = *(d->indicePerCalcoloNext);
            if(indiceDaCalcolare == d->grafo->N){
                pthread_mutex_unlock(m);
                break;
            }
            *(d->indicePerCalcoloNext)+=1;
            pthread_mutex_unlock(m);

            double sommatoriaYt = 0;
            
            // Sommo ogni y[i] (y[i-1]) con i nodo entrante in indiceDaCalcolare
            inmap *arr = d->grafo->in[indiceDaCalcolare];
            int arraySize = d->grafo->in[indiceDaCalcolare]->size;
            for(int i=0;i<arraySize;i++)
                sommatoriaYt += d->y[arr->data[i]-1];


            // Calcolo elemento indiceDaCalcolare di xnext
            d->xnext[indiceDaCalcolare] = d->teleport + *(d->DE) + (d->d*sommatoriaYt);

            sommatoriaTemporaneaErrore += fabs(d->xnext[indiceDaCalcolare] - d->x[indiceDaCalcolare]);

        }

        pthread_mutex_lock(m);
        *(d->errore) += sommatoriaTemporaneaErrore;
        pthread_mutex_unlock(m);



        pthread_mutex_lock(d->mFase);
        *(d->tf2) += 1;
        if(*(d->tf2) == d->taux)
            pthread_cond_signal(d->cond_fineFase2);
        // Aspetto che main esca dal wait e abbia calcolato DE
        pthread_cond_wait(d->cond_fineFase1,d->mFase);
        pthread_mutex_unlock(d->mFase);

    }


    return (void *)NULL;
}



void *bodyHandler(void *arg)
{
    datiHandler *d = (datiHandler *)arg;
    // Creo maschera
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    sigaddset(&mask, SIGUSR2);
    
    // Creo var per segnale
    int s = 0;

    // fprintf(stderr,"Se desideri visualizzare l'attuale rank piú alto il mio id é: %d\n",getpid());

    // Controllo fino a quando l'esecuzione non termina
    while(!(*d->esecuzioneTerminata))
    {
        int e = sigwait(&mask, &s);
        if(e!=0)
            fprintf(stderr,"Errore di gestione del segnale\n");
        // Se ricevo sigusr1 stampo rank massimo
        if(s == SIGUSR1)
        {
            // Uso il primo el. per l'inizializzazione
            int indiceRankMassimo = 0;
            double valoreRankMassimo = d->x[indiceRankMassimo];
            
            for(int i=0; i<d->numeroNodi; i++)
            {
                if(d->x[i] > valoreRankMassimo)
                {
                    valoreRankMassimo = d->x[i];
                    indiceRankMassimo = i;
                }
            }
            fprintf(stderr,"Iterazione numero: %d\n",*d->numeroIterazione);
            fprintf(stderr,"Nodo con il massimo pagerank: %d\n",indiceRankMassimo);
            fprintf(stderr,"Valore: %f\n",valoreRankMassimo);
        }
        // Quando l'esecuzione termina, termino
        else if(s == SIGUSR2)
            break;
    }

    return (void *)NULL;
}
