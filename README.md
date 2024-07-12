# Parallelizzazione calcolo Pagerank

## Inizializzazione
Inizialmente si ha la fase di inizializzazione del vettore x, durante la quale **ad ogni thread viene assegnato un blocco di indici** dove inseriranno il valore iniziale (valorePasso0)

## Iterazione
Le iterazioni sono composte da 2 fasi:

### Thread Ausiliari

- **Fase 1**: Utilizzando la precedente divisione in blocchi del vettore x, ogni thread ne calcola la rispettiva sommatoria del contributo dei nodi dead-end e definisce gli elementi del vettore y. Alla fine della scansione, ogni thread, mediante un mutex, somma la propria parte di sommatoria (temporanea) in quella principale. Una volta fatto, segnalano di aver terminato la prima parte tramite una variabile di condizione, e si mettono in attesa su una seconda. *Si noti che il segnale viene inviato solo una volta che tutti i thread hanno terminato, semplicemente utilizzando un contatore*.

- **Fase 2**: Una volta che gli ausiliari sono tutti in attesa, il main thread manda un segnale broadcast, iniziando la seconda parte. Qui viene calcolato il vettore xnext e la sommatoria dell'errore nel seguente modo. Viene utilizzato un indice condiviso tra gli ausiliari, che rappresenta l'elemento del vettore xnext da calcolare, protetto naturalmente da un mutex (si usa lo stesso della fase precedente). Quindi, ogni thread, ne leggerá l'indice prima di incrementarlo di 1. Ottenuto l'indice viene liberato il mutex, per permettere agli altri ausiliari di continuare, e inizia il calcolo dell'elemento, quale, una volta terminato, viene utilizzato per la sommatoria dell'errore, facendo la differenza con l'elemento alla stessa posizione del vettore x.
 La fase termina quando l'indice é uguale al numero dei nodi, l'ultimo elemento calcolato sará stato quindi quello in posizione numeroNodi - 1. Tramite mutex ogni ausiliare aggiunge la sua parte di sommatoria dell'errore a quella principale, per poi utilizzare il metodo descritto precedentemente per segnalare il main thread e mettersi in attesa.

 ### Main thread

 - **Fase 1**: Nella prima fase, il main thread si mette in attesa che gli ausiliari terminino. Quando viene segnalato, imposta a 0 il contatore 'tf1' che rappresenta i thread che hanno segnalato (per l'iterazione successiva), calcola il valore del dead-end grazie alla sommatoria completa e imposta a 0 l'errore, che sta per essere calcolato. Invia il segnale di broadcast per iniziare la fase 2 e si mette in attesa che termini.

 - **Fase 2**: Una volta segnalato, imposta a 0 'tf2', l'indice per il calcolo degli elementi xnext e la sommatoria del dead-end. Scambia poi i puntatori di x e xnext per l'iterazione successiva e controlla se l'algoritmo deve terminare. In caso positivo, imposta la variabile condivisa 'stop' a true (quale fará terminare gli ausiliari all'inizio della prossima iterazione) e quindi invia il segnale broadcast. Altrimenti invia solo il segnale, e comincia l'iterazione successiva.