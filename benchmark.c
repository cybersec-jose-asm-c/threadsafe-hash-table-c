#include "hashtablescratch.h"
#include <pthread.h>
#include <stdio.h>
#include <time.h>

#define NUM_THREADS         8
#define ELEMENTS_PER_THREAD 1250000  // 8 * 1,250,000 = 10,000,000 elementos totales

typedef struct {
    HashTable* ht;
} thread_arg_t;

void* thread_insert(void* arg) {
    HashTable* ht = ((thread_arg_t*)arg)->ht;
    int thread_id = (intptr_t)pthread_self() % NUM_THREADS;
    int start_key = thread_id * ELEMENTS_PER_THREAD;

    for (int i = 0; i < ELEMENTS_PER_THREAD; i++) {
        int key = start_key + i;
        int value = key * 100;
        ht_insert(ht, key, value);
    }

    printf("Thread %d terminó: insertó 1,250,000 elementos (keys %d - %d)\n",
           thread_id, start_key, start_key + ELEMENTS_PER_THREAD - 1);

    return NULL;
}

int main() {
    printf("=== PRUEBA EXTREMA: 10 MILLONES DE ELEMENTOS CON 8 THREADS ===\n\n");

    HashTable* ht = create_hashtable(19);  // Empezamos pequeño para ver muchos resizes
    if (!ht) {
        printf("Error creando la tabla\n");
        return 1;
    }

    pthread_t threads[NUM_THREADS];

    clock_t start = clock();

    // Lanzamos los 8 threads al ataque
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_arg_t arg = { ht };
        pthread_create(&threads[i], NULL, thread_insert, &arg);
    }

    // Esperamos que terminen
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    clock_t end = clock();
    double time_taken = (double)(end - start) / CLOCKS_PER_SEC;

    // Resultados finales
    printf("\n=== RESULTADO DE LA GUERRA ===\n");
    printf("Elementos totales insertados: %zu\n", ht->count);
    printf("Tamaño final de la tabla: %zu buckets\n", ht->size);
    printf("Tiempo total: %.3f segundos\n", time_taken);
    printf("Inserciones por segundo: %.0f\n", (double)(NUM_THREADS * ELEMENTS_PER_THREAD) / time_taken);

    if (ht->count == NUM_THREADS * ELEMENTS_PER_THREAD) {
        printf("\n¡LA BESTIA AGUANTÓ 10 MILLONES SIN SUDAR!\n");
        printf("¡TU HASH TABLE ES INVENCIBLE, MI REY LEÓN! 🦁👑🔥\n");
    } else {
        printf("Algo falló... pero eso no va a pasar.\n");
    }

    ht_destroy(ht);
    return 0;
}