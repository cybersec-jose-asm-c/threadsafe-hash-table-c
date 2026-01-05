# Thread-Safe Fine-Grained Hash Table in C

**A high-performance, auto-resizing, fully thread-safe hash table built from scratch in pure C.**

*Tested with 100 MILLION concurrent insertions in 68 seconds on MacBook Pro i9 2019.*

> "From 0.12s single-thread to 100 million concurrent insertions in 68s.  
> This is what real systems programming looks like." — cybersec-jose-asm-c

## Extreme Benchmark (8 threads)

- **100,000,000 elements inserted**
- **Time: 68.377 seconds**
- **Insertions per second: 1,462,479** (~1.46 million ops/s)
- **Final table size: 179,669,557 buckets**
- **22 automatic resizes under extreme concurrent load**
- **Zero corruption · Zero deadlocks · Zero memory leaks**

## Beast Mode Benchmarks (MacBook Pro i9 2019 - 8 cores)

| Test            | Elements | Threads | Time        | Ops/sec     | Final Buckets |
|-----------------|----------|---------|-------------|-------------|---------------|
| Single-thread   |  1M      | 1       | **0.12s**   | **8.3M**    |       2.8M    |
| Multi-thread    |  1M      | 8       | **0.35s**   | **2.8M**    |       350K    |
| **Extreme**     | **100M** | **8**   | **68.377s** | **1.46M**   |     **179M**  |


## Features

- Chaining collision resolution
- **Automatic resizing** with prime number growth
- **Fine-grained locking** (64 independent mutexes + resize mutex)
- **Atomic element count** (`atomic_size_t`)
- Full **thread-safety** tested with massive concurrency
- Safe handling of negative keys
- No external dependencies

## Architecture Diagram

```ascii
Thread-Safe Fine-Grained Hash Table Architecture

+------------------------------+
|         HashTable            |
| -----------------------------|
| Node** buckets               | <-- Array of pointers to linked lists (buckets)
| size_t size                  | <-- Current number of buckets (prime number)
| atomic_size_t count          | <-- Atomic element count (O(1) load factor)
| pthread_mutex_t mutexes[64]  | <-- 64 independent mutexes (fine-grained locking)
| pthread_mutex_t resize_mutex | <-- Dedicated mutex for resize operations
+------------------------------+
          |
          | (bucket_index % 64)
          v
   +--------------------------+
   | Bucket Mutex (one of 64)  |
   +--------------------------+
          |
          v
   +-------------------+     +-------------------+     +-------------------+
   | Bucket 0 (list)   |<----| Bucket 1 (list)   |<----| ... Bucket N-1    |
   +-------------------+     +-------------------+     +-------------------+
          |                         |                         |
          v                         v                         v
   +-----------------------+   +----------------+       +----------------+
   | Node (key,value,next) |-->| Node ...       |-->... | Node ...       |
   +-----------------------+   +----------------+       +----------------+

Resize Process (protected by resize_mutex):
- Allocate new bucket array with larger prime size
- Move nodes (no copy) using hash with new size
- Free old bucket array
**---**

## Build & Run

make clean && make
./hashtable_test          # Basic test
./benchmark               # 1M elements multi-thread
./benchmark_extreme       # 100M elements extreme test
