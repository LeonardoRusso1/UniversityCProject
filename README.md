This is a brief report that describes how I parallelized the Pagerank calculation.

# Pagerank Calculation Parallelization

## Initialization
Initially, the vector x undergoes an initialization phase, during which **each thread is assigned a block of indices** where they will insert the initial value (valorePasso0).

## Iteration
The iterations consist of 2 phases:

### Auxiliary Threads

- **Phase 1**: Using the previous division into blocks of vector x, each thread calculates its respective sum of the contributions from dead-end nodes and defines the elements of vector y. At the end of the scan, each thread, using a mutex, adds its part of the (temporary) sum to the main sum. Once done, they signal that they have completed the first part via a condition variable and wait on a second one. *Note that the signal is sent only once all threads have finished, simply using a counter*.

- **Phase 2**: Once all the auxiliary threads are waiting, the main thread sends a broadcast signal, starting the second part. Here, vector xnext and the sum of the error are calculated as follows. A shared index among the auxiliary threads is used, representing the element of vector xnext to be calculated, protected by a mutex (the same one used in the previous phase). Each thread reads the index before incrementing it by 1. Once the index is obtained, the mutex is released to allow other auxiliaries to proceed, and the calculation of the element begins. Once completed, the element is used to sum the error by taking the difference with the element at the same position in vector x.
The phase ends when the index equals the number of nodes, meaning the last calculated element will be the one at position numberOfNodes - 1. Using a mutex, each auxiliary adds its part of the error sum to the main one, then uses the previously described method to signal the main thread and wait.

### Main Thread

- **Phase 1**: In the first phase, the main thread waits for the auxiliaries to finish. When signaled, it sets the counter 'tf1' representing the threads that have signaled (for the next iteration) to 0, calculates the dead-end value using the complete sum, and sets the error to 0, which is about to be calculated. It sends the broadcast signal to start phase 2 and waits for it to finish.

- **Phase 2**: Once signaled, it sets 'tf2', the index for calculating the xnext elements, and the dead-end sum to 0. Then it swaps the pointers of x and xnext for the next iteration and checks if the algorithm should terminate. If so, it sets the shared variable 'stop' to true (which will stop the auxiliaries at the beginning of the next iteration) and then sends the broadcast signal. Otherwise, it simply sends the signal and begins the next iteration.
