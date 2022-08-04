#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include <unistd.h>
#include <limits.h>
#include <sys/mman.h>
#include<errno.h>
#include<string.h>

#ifndef foo_h__
#define foo_h__

typedef struct block_info
{
   int size;
   struct block_info *next;
}block_info;

__thread block_info *bin_8     = NULL;
__thread block_info *bin_64    = NULL;
__thread block_info *bin_512   = NULL;
__thread block_info *bin_large = NULL;

__thread void *thread_unused_heap_start = NULL;
__thread void *thread_heap_end = NULL;

void *heap_used_memory_end = NULL;

pthread_mutex_t global_heap_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t stats_mutex = PTHREAD_MUTEX_INITIALIZER;

unsigned long total_number_of_blocks = 0;
unsigned long total_arena_size_allocated = 0;
unsigned long total_mmap_size_allocated = 0;
unsigned long total_allocation_request = 0;
unsigned long total_free_blocks = 0;
unsigned long total_free_request = 0;

//function declarations

void * heap_allocate(size_t size);
block_info ** get_bin(size_t size);
void * block_from_unused_heap(size_t size);
void * align8(void *x);

void * alloc_large(size_t size);
void * find_best_fit_from_bin_large(size_t size);
void * mmap_new_memory(size_t size);

void * malloc_new(size_t);
void free_new(void *p);
 
#endif  // foo_h__