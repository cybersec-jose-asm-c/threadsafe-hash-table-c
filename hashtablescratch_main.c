#include "hashtablescratch.h"
#include <pthread.h>
#include <stdio.h>
#include <time.h>

#define NUM_THREADS         8                    // Number of threads to launch
#define ELEMENTS_PER_THREAD 1000000               // Elements each thread will insert
                                                 // Total: 8 * 125000 = 1,000,000 elements

// Structure passed to each thread
typedef struct {
    HashTable* ht;       // Pointer to the shared hash table
    int thread_id;       // Thread identifier (0 to NUM_THREADS-1)
} thread_arg_t;

// Function executed by each thread
void* thread_insert(void* arg) {
    thread_arg_t* data = (thread_arg_t*)arg;
    HashTable* ht = data->ht;
    int start_key = data->thread_id * ELEMENTS_PER_THREAD;  // Unique key range per thread

    // Each thread inserts its own block of keys
    for (int i = 0; i < ELEMENTS_PER_THREAD; i++) {
        int key = start_key + i;
        int value = key * 100;  // Arbitrary value, easy to identify
        ht_insert(ht, key, value);
    }

    printf("Thread %d finished: inserted %d elements (keys %d to %d)\n",
           data->thread_id, ELEMENTS_PER_THREAD, start_key, start_key + ELEMENTS_PER_THREAD - 1);

    return NULL;
}

int main(void) {
    printf("=== FINE-GRAINED THREAD-SAFE HASH TABLE TEST ===\n\n");

    // Create hash table with small initial size to force multiple resizes
    HashTable* ht = create_hashtable(INITIAL_TABLE_SIZE);
    if (!ht) {
        printf("Error creating hash table\n");
        return 1;
    }

    pthread_t threads[NUM_THREADS];
    thread_arg_t args[NUM_THREADS];

    clock_t start = clock();  // Start timing

    // Launch all threads
    for (int i = 0; i < NUM_THREADS; i++) { // Initialize args and create thread
        args[i].ht = ht;                    // Shared hash table
        args[i].thread_id = i;              // Thread ID
        pthread_create(&threads[i], NULL, thread_insert, &args[i]); // Create thread
    }

    // Wait for all threads to finish
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    clock_t end = clock();
    double time_taken = (double)(end - start) / CLOCKS_PER_SEC;

    // Final results
    printf("\n=== FINAL RESULTS ===\n");
    printf("Threads used: %d\n", NUM_THREADS);
    printf("Elements inserted per thread: %d\n", ELEMENTS_PER_THREAD);
    printf("Total expected elements: %d\n", NUM_THREADS * ELEMENTS_PER_THREAD);
    printf("Actual elements in table: %zu\n", ht->count);
    printf("Final table size: %zu buckets\n", ht->size);
    printf("Total time: %.3f seconds\n", time_taken);
    printf("Insertions per second: %.0f\n", (double)(NUM_THREADS * ELEMENTS_PER_THREAD) / time_taken);

    if (ht->count == NUM_THREADS * ELEMENTS_PER_THREAD) {
        printf("\n¡ABSOLUTE SUCCESS! Fine-grained thread-safety works perfectly.\n");
        printf("¡Your hash table is a MULTI-THREAD BEAST! 🦁🔥\n");
    } else {
        printf("\nError: data corruption or loss.\n");
    }

    // Clean up
    ht_destroy(ht);

    return 0;
}
