#include <stdio.h>
#include <time.h>      // ← Esto es lo que causaba el error: para time() y srand()
#include <math.h>    // ← Esto es necesario para sqrt()
#include <stdint.h>
#include <string.h>
#include <stdlib.h>    // ← Esto faltaba: para malloc, calloc, etc.
#include <unistd.h>  // ← Esto es necesario para sleep()
#include <stdbool.h>  // ← Esto es necesario para usar bool, true, false
#include <pthread.h>
#include "hashtablescratch.h"


// Function prototypes for static functions
static size_t hash_function(int key, size_t table_size);
static bool is_prime(size_t n);
static size_t next_prime(size_t n);
static pthread_mutex_t* get_bucket_mutex(HashTable* table, size_t bucket_index);



// ============================================================================================= //
// ======================================= BUCKET MUTEX ======================================== //
// ============================================================================================= //
static pthread_mutex_t* get_bucket_mutex(HashTable* table, size_t bucket_index) {
    return &table->mutexes[bucket_index % NUM_MUTEXES];
}


// ============================================================================================= //
// ======================================== HASH FUNCTION ====================================== //
// ============================================================================================= //
static size_t hash_function(int key, size_t table_size) {
    return ((long long)key % (long long)table_size + table_size) % table_size;
}


// ============================================================================================= //
// ============================================ RESIZE ========================================= //
// ============================================================================================= //
void ht_resize(HashTable* table, size_t new_size) {
    if (!table || new_size == 0 || new_size <= table->size) return;

    Node** new_buckets = calloc(new_size, sizeof(Node*));
    if (!new_buckets) return;
    
    // Rehash all existing keys into new buckets
    for (size_t i = 0; i < table->size; i++) {
        Node* current = table->buckets[i];
        while (current) { // Traverse linked list until NULL
            Node* next_node = current->next; // Set next node before re-linking, setting to NULL
            
            size_t new_index = hash_function(current->key, new_size);
            
            // Insert at the beginning of the new bucket
            current->next = new_buckets[new_index]; // Set to NULL if empty or current head
            
            new_buckets[new_index] = current; // Update head to current node

            current = next_node; // Move to next node
        }
    }
    // Free old buckets and update table
    free(table->buckets);

    table->buckets = new_buckets;
    table->size = new_size;

}


// ============================================================================================= //
// =========================================== CREATE ========================================== //
// ============================================================================================= //
HashTable* create_hashtable(size_t size) {
    HashTable* table = malloc(sizeof(HashTable));
    if (!table) return NULL;

    table->size = size;
    table->buckets = calloc(table->size, sizeof(Node*));
    
    
    // If calloc fails then it frees the memory allocated for the table
    if (!table->buckets) {
        free(table);
        return NULL;
    }

    atomic_init(&table->count, 0);

    // Initialize mutexes
    for (int i = 0; i < NUM_MUTEXES; i++) {
        if (pthread_mutex_init(&table->mutexes[i], NULL) != 0) {
            // Limpieza parcial
            for (int j = 0; j < i; j++) pthread_mutex_destroy(&table->mutexes[j]);
            free(table->buckets);
            free(table);
            return NULL;
        }
    }

    // Initialize resize mutex
    if (pthread_mutex_init(&table->resize_mutex, NULL) != 0) {
        for (int i = 0; i < NUM_MUTEXES; i++) pthread_mutex_destroy(&table->mutexes[i]);
        free(table->buckets);
        free(table);
        return NULL;
    }

    return table;
}


// ============================================================================================= //
// ============================================= INSERT ======================================== //
// ============================================================================================= //
void ht_insert(HashTable* table, int key, int value) {
    if (!table) return;

    size_t bucket_index = hash_function(key, table->size);
    pthread_mutex_t* bucket_mutex = get_bucket_mutex(table, bucket_index);

    pthread_mutex_lock(bucket_mutex);

    // Search if key already exists and update value if so
    Node* current = table->buckets[bucket_index];
    while (current) {
        if (current->key == key) {
            current->value = value;
            pthread_mutex_unlock(bucket_mutex);
            return;
        }
        current = current->next;
    }

    // Key does not exist: create new node
    Node* new_node = malloc(sizeof(Node));
    if (!new_node) {
        pthread_mutex_unlock(bucket_mutex);
        return;
    }
    new_node->key = key;
    new_node->value = value;
    new_node->next = table->buckets[bucket_index];
    table->buckets[bucket_index] = new_node;

    table->count++;
    
    // Check load factor
    float load_factor = (float)table->count / (float)table->size;
    if (load_factor > 0.7f) {
        pthread_mutex_unlock(bucket_mutex);  // Release bucket lock

        pthread_mutex_lock(&table->resize_mutex);  // Acquire resize lock

        // Double-check load factor (in case another thread resized)
        if ((float)table->count / (float)table->size > 0.7f) {
            size_t candidate = table->size * 2 + 1;
            size_t new_size = next_prime(candidate);
            printf("Resizing table from %zu to %zu due to load factor %.2f\n", table->size, new_size, load_factor);
            ht_resize(table, new_size);
        }

        pthread_mutex_unlock(&table->resize_mutex);  // Release resize lock
    } else {
        pthread_mutex_unlock(bucket_mutex);  // Release bucket lock if no resize
    }
}


// ============================================================================================= //
// ============================================= GET =========================================== //
// ============================================================================================= //
int ht_get(HashTable* table, int key_to_seek, int* seeked_value) {
    if (!table || !table->buckets) return 0;

    size_t bucket_index = hash_function(key_to_seek, table->size);
    pthread_mutex_t* mutex = &table->mutexes[bucket_index % NUM_MUTEXES];

    pthread_mutex_lock(mutex);
    
    Node* current = table->buckets[bucket_index];
    while (current) {
        if (current->key == key_to_seek) {
            *seeked_value = current->value;
            pthread_mutex_unlock(mutex);
            return 1; // Found
        }
        current = current->next;
    }
    
    pthread_mutex_unlock(mutex);
    return 0; // Not found
}


// ============================================================================================= //
// ============================================ PRINT ========================================== //
// ============================================================================================= //
void print_hashtable(HashTable* table) {
    if (!table) return;

    pthread_mutex_lock(&table->resize_mutex); // Lock during print to avoid resizing

    for (size_t i = 0; i < table->size; i++) {
        printf("Bucket[%zu]: ", i);
        Node* current = table->buckets[i];
        while (current) {
            printf("-> ( %d, %d) ", current->key, current->value);
            current = current->next;
        }
        printf("-> NULL\n");
    }

    pthread_mutex_unlock(&table->resize_mutex); // Unlock after printing
}


// ============================================================================================= //
void ht_delete(HashTable* table, int key) {
    if (!table) return;

    size_t bucket_index = hash_function(key, table->size);
    pthread_mutex_t* mutex = get_bucket_mutex(table, bucket_index);

    pthread_mutex_lock(mutex);


    Node* current = table->buckets[bucket_index];
    Node* prev = NULL;
    // If first node is the one to delete
    while (current && current->key == key) {
        Node* temp = current;
        current = current->next;
        free(temp);
        
        table->count--;
        
        table->buckets[bucket_index] = current; // Update head
        pthread_mutex_unlock(mutex);
        return;
    }
    // Search for the key in the linked list
    while (current) {
        if (current->key == key) {
            prev->next = current->next;
            free(current);
            
            table->count--;
            
            pthread_mutex_unlock(mutex);
            return;
        }
        prev = current;
        current = current->next;
    }
    pthread_mutex_unlock(mutex);
}


// ============================================================================================= //
// ============================================ COUNT ========================================= //
// ============================================================================================= //
size_t ht_count(const HashTable* table) {
    if (!table) return 0;

    size_t count = 0;
    for (size_t i = 0; i < table->size; i++) {
        Node* current = table->buckets[i];
        while (current) {
            count++;
            current = current->next;
        }
    }
    return count;
}


// ============================================================================================= //
// =========================================== DESTROY ======================================== //
// ============================================================================================= //
void ht_destroy(HashTable* table) {
    if (!table) return;

    for (size_t i = 0; i < table->size; i++) {
        Node* current = table->buckets[i];
        while (current) {
            Node* temp = current;
            current = current->next;
            free(temp);
        }
    }

    free(table->buckets);
    table->buckets = NULL;
    
    for (int i = 0; i < NUM_MUTEXES; i++) {
        pthread_mutex_destroy(&table->mutexes[i]);
    }

    pthread_mutex_destroy(&table->resize_mutex); // Destroy resize mutex

    free(table);
    table = NULL;
    
}

// ================================================================================ 
// ========================= FUNCIONES ADICIONALES ================================
// ================================================================================
static bool is_prime(size_t n) {
    if (n <= 1) return false;
    if (n <= 3) return true;
    if (n % 2 == 0 || n % 3 == 0) return false;

    // Checa divisores de la forma 6k ± 1 hasta sqrt(n)
    size_t i;
    for (i = 5; i * i <= n; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0) {
            return false;
        }
    }
    return true;
}



// Función principal: siguiente primo >= n
static size_t next_prime(size_t n) {
    if (n <= 2) return 2;

    // Si n es par, empieza desde el siguiente impar
    if (n % 2 == 0) n++;

    // Busca el siguiente número impar que sea primo
    while (!is_prime(n)) n += 2;  // salta pares

    return n;
}
