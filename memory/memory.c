#include "memory.h"

void * find_best_fit_from_bin_large(size_t size)
{
    block_info *b = bin_large;
    block_info *best_fit = NULL;
    int min_fit_size = INT_MAX;
    void *ret = NULL;

    while(b != NULL)
    {
         if(b->size >= size && b->size < min_fit_size)
         {
            best_fit = b;
            min_fit_size = b->size;
         }
         b = b->next;
    }

    /*If best fit found, update list*/
    if(NULL != best_fit)
    {
        // if best_fit is first block.
        if (best_fit == bin_large)
        {
           bin_large = bin_large->next;
           best_fit->next = NULL;
           ret = (void *)((void *)best_fit + sizeof(block_info));
        }
        else
        {
          b = bin_large;
          while(b != NULL && b->next != best_fit)
          {
            b = b->next;
          }
          if(b != NULL)
          {
             b->next = best_fit->next;
          }
          best_fit->next = NULL;
          ret = (void *)((void *)best_fit + sizeof(block_info));
        }
    }

    return ret;
}

void * mmap_new_memory(size_t size)
{
    int num_pages =
        ((size + sizeof(block_info) - 1)/sysconf(_SC_PAGESIZE)) + 1;
    int required_page_size = sysconf(_SC_PAGESIZE) * num_pages;

    void *ret = mmap(NULL, // let kernel decide.
                     required_page_size,
                     PROT_READ | PROT_WRITE,
                     MAP_ANONYMOUS| MAP_PRIVATE,
                     -1, //no file descriptor
                     0); //offset.

    block_info b;
    b.size = (required_page_size - sizeof(block_info));
    b.next = NULL;

    ret = memcpy(ret, &b, sizeof(block_info));
    ret = ((char*)ret + sizeof(block_info));

    // update stats variables.
    pthread_mutex_lock(&stats_mutex);
    total_mmap_size_allocated += size;
    pthread_mutex_unlock(&stats_mutex);

    return ret;
}

void *alloc_large(size_t size)
{
   void * ret = NULL;
   if(NULL != bin_large)
   {
       //pthread_mutex_lock(&global_heap_mutex);
       ret = find_best_fit_from_bin_large(size);
      // pthread_mutex_unlock(&global_heap_mutex);
   }

   /*either bin_large is empty or no best fit was found.*/
   if(ret == NULL)
   {
       //pthread_mutex_lock(&global_heap_mutex);
       ret = mmap_new_memory(size);
      // pthread_mutex_unlock(&global_heap_mutex);
   }
    return ret;
}

/*----------------------------------------------------------------*/

void *align8(void *x)
{
    unsigned long p = (unsigned long)x;
    p = (((p - 1) >> 3) << 3) + 8;
    return (void *)p;
}

block_info** get_bin(size_t size)
{
    switch(size)
    {
       case 8   : return &bin_8;
       case 64  : return &bin_64;
       case 512 : return &bin_512;
       default  : return &bin_large;
    }
}

void * block_from_unused_heap(size_t size)
{
    if(NULL == thread_unused_heap_start ||
       (thread_heap_end - thread_unused_heap_start) <
           (size + sizeof(block_info)))
    {

        if(NULL == heap_used_memory_end ||
            (sbrk(0) - heap_used_memory_end) < (size + sizeof(block_info)))
        {
            if(NULL == heap_used_memory_end)
            {
                heap_used_memory_end = sbrk(0);
                if(heap_used_memory_end == (void*) -1)
                {
                    errno = ENOMEM;
                    perror("\n sbrk(0) failed.");
                    return NULL;
                }
            }

            align8(heap_used_memory_end);
        }


        if(sbrk(sysconf(_SC_PAGESIZE) * 100) == (void *) -1)
        {
            errno = ENOMEM;
            perror("\n sbrk failed to extend heap.");
            return NULL;
        }

        thread_unused_heap_start = heap_used_memory_end;
        thread_heap_end =
            heap_used_memory_end + (sysconf(_SC_PAGESIZE));
        heap_used_memory_end =  thread_heap_end;
    }

    block_info b;
    b.size = size;
    b.next = NULL;

    memcpy(thread_unused_heap_start, &b, sizeof(block_info));
    thread_unused_heap_start += (sizeof(block_info) + size);

    pthread_mutex_lock(&stats_mutex);
    total_number_of_blocks++;
    total_arena_size_allocated += size;
    pthread_mutex_unlock(&stats_mutex);

    return (thread_unused_heap_start - size);
}

void *heap_allocate(size_t size)
{

   block_info **bin = get_bin(size);
   void * ret = NULL;
   
   if(NULL != *bin)
   {
       block_info *p = *bin;
       *bin =  p->next;
       p->next = NULL;

       pthread_mutex_lock(&stats_mutex);
       total_free_blocks--;
       pthread_mutex_unlock(&stats_mutex);
       ret = (void *)((char*)p + sizeof(block_info));
   }
   else 
   {     pthread_mutex_lock(&global_heap_mutex);
         ret =  block_from_unused_heap(size);
         pthread_mutex_unlock(&global_heap_mutex);
   }

   return ret;
}


void* malloc_new(size_t size)
{
     pthread_mutex_lock(&stats_mutex);
     total_allocation_request++;
     pthread_mutex_unlock(&stats_mutex);

     void * ret = NULL;

     if(size < 0)
     {
        perror("\n Invalid memory request.");
        return NULL;
     }

     if(size > 512)
     {  
        ret = alloc_large(size);
     }
     else
     {
       size = (size <= 8)? 8 : ((size<=64)? 64: 512);
       ret = heap_allocate(size);
     }
     return ret;
}

void free_new(void *p)
{
   //update stats variables.
   pthread_mutex_lock(&stats_mutex);
   total_free_request++;
   total_free_blocks++;
   pthread_mutex_unlock(&stats_mutex);

   if(NULL != p)
   {
      block_info *block  = (block_info *)(p - sizeof(block_info));
      memset(p, '\0', block->size);

      block_info **bin = get_bin(block->size);
      block_info *check_bin = *bin;

      while(check_bin != NULL)
      {
         if(check_bin == block)
         {
            return;
         }
         check_bin = check_bin->next;
      }

      block->next = *bin;
      *bin = block;
    }
}