#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdatomic.h>

#define INITIAL_TABLE_SIZE 19
#define NUM_MUTEXES 64  // Number of mutexes for finer-grained locking

// Node structure for linked list in each bucket
typedef struct Node {
    int key;
    int value;
    struct Node* next;
} Node;

// HashTable structure
typedef struct HashTable {
    Node** buckets;
    size_t size;
    atomic_size_t count; // Use atomic for thread-safe count
    pthread_mutex_t mutexes[NUM_MUTEXES]; // Mutex for thread safety
    pthread_mutex_t resize_mutex; // Mutex for resizing    
} HashTable;


// Function prototypes
HashTable* create_hashtable(size_t size);
void ht_insert(HashTable* table, int key, int value);
int ht_get(HashTable* table, int key_to_seek, int* seeked_value);
void ht_delete(HashTable* table, int key);
size_t ht_count(const HashTable* table);
void ht_destroy(HashTable* table);
void print_hashtable(HashTable* table);
void ht_resize(HashTable* table, size_t new_size);

#endif // HASHTABLE_H