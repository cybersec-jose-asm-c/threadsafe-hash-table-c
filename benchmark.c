#include "hashtablescratch.h"
#include <pthread.h>
#include <stdio.h>
#include <time.h>

#define NUM_THREADS         8
#define ELEMENTS_PER_THREAD 12500000  // 8 * 1,250,000 = 10,000,000 elementos totales

typedef struct {
    HashTable* ht;
    int thread_id;
} thread_arg_t;

void* thread_insert(void* arg) {
    thread_arg_t* data = (thread_arg_t*)arg;
    HashTable* ht = data->ht;
    int thread_id = data->thread_id;
    int start_key = thread_id * ELEMENTS_PER_THREAD;

    for (int i = 0; i < ELEMENTS_PER_THREAD; i++) {
        ht_insert(ht, start_key + i, (start_key + i) * 100);
    }

    printf("Thread %d terminó: insertó %d elementos\n", thread_id, ELEMENTS_PER_THREAD);
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
    thread_arg_t args[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; i++) {
        args[i].ht = ht;
        args[i].thread_id = i;
        pthread_create(&threads[i], NULL, thread_insert, &args[i]);
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
        printf("\n¡LA BESTIA AGUANTÓ %zu MILLONES SIN SUDAR!\n", ht->count);
        printf("¡TU HASH TABLE ES INVENCIBLE, MI REY LEÓN! 🦁👑🔥\n");
    } else {
        printf("Algo falló... pero eso no va a pasar.\n");
    }

    ht_destroy(ht);
    return 0;
}