# Thread-Safe Fine-Grained Hash Table in C

**A high-performance, auto-resizing, fully thread-safe hash table implemented in pure C — from scratch.**

Built with passion, sweat, and pure systems programming love.

## Beast Mode Performance (MacBook Pro i9 2019 - 8 cores / 16 threads)

- **10,000,000 concurrent insertions** with 8 threads
- **Time: 3.858 seconds**
- **Insertions per second: 2,591,952** (~2.6 million ops/s)
- **Final table size: 2,807,303 buckets**
- **17 automatic resizes under extreme concurrent load**
- **Zero data corruption · Zero deadlocks · Zero memory leaks**


## Features

- Chaining collision resolution
- **Automatic resizing** with prime number growth for optimal distribution
- **Fine-grained locking** (64 independent mutexes + dedicated resize mutex)
- **Atomic element count** (`atomic_size_t`) for O(1) load factor
- Full **thread-safety** tested with massive concurrent workloads
- Safe handling of negative keys
- No external dependencies — pure standard C + pthreads
- Clean modular design (.h + .c)

## Build & Run

```bash
make clean && make
./hashtable_test          # Basic functionality test
./benchmark               # 1M elements multi-thread benchmark
./benchmark_extreme       # 10M elements extreme test (the beast mode)


Thread-Safe Fine-Grained Hash Table Architecture

+---------------------------+
|       HashTable           |
| ------------------------- |
| Node** buckets            |  <-- Array de punteros a listas enlazadas (buckets)
| size_t size               |  <-- Número actual de buckets (primo)
| atomic_size_t count       |  <-- Conteo atómico de elementos (O(1) load factor)
| pthread_mutex_t mutexes[64]  <-- 64 mutexes independientes (fine-grained locking)
| pthread_mutex_t resize_mutex  <-- Mutex dedicado solo para resize
+---------------------------+
          |
          | (bucket_index % 64)
          v
   +-------------------+
   | Bucket Mutex (uno de 64) |
   +-------------------+
          |
          v
   +-------------------+     +-------------------+     +-------------------+
   | Bucket 0 (lista)  |<----| Bucket 1 (lista)  |<----| ... Bucket N-1    |
   +-------------------+     +-------------------+     +-------------------+
          |                         |                         |
          v                         v                         v
   +----------------+        +----------------+        +----------------+
   | Node (key,value,next) |-->| Node ...       |-->... | Node ...       |
   +----------------+        +----------------+        +----------------+

Resize Process (protegido por resize_mutex):
- Crea nuevo array de buckets con tamaño primo mayor
- Mueve nodos (no copia) usando hash con new_size
- Libera viejo array

Inserción:
- Lock bucket mutex específico
- Inserta/actualiza nodo
- Incrementa count (atómico)
- Si load_factor > 0.7 → trylock resize_mutex → resize si necesario
