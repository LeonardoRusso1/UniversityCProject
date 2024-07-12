CC=gcc
CFLAGS=-std=c11 -Wall -g -O3 -pthread
LDLIBS=-lm -lrt -pthread

# Lista dei file sorgente (.c)
SOURCES=main.c utilityFunctions.c set.c threads.c pagerank.c

# Lista dei file oggetto (.o) generati dalla compilazione dei file sorgente
OBJECTS=$(SOURCES:.c=.o)

# Nome dell'eseguibile
EXECUTABLE=pagerank

all: $(EXECUTABLE)

# Regola per compilare l'eseguibile utilizzando tutti i file oggetto
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@ $(LDLIBS)

# Regola generica per compilare un file oggetto da un file sorgente
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Dipendenze dei file oggetto dai file di intestazione
main.o: dataStructures.h utilityFunctions.h
utilityFunctions.o: dataStructures.h
set.o: set.h dataStructures.h
threads.o: threads.h
pagerank.o: pagerank.h

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)
