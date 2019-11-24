/*  

    Copyright 2018-19 by

    University of Alaska Anchorage, College of Engineering.

    All rights reserved.

    Contributors:
		   Kristopher Carroll and
		   Christoph Lauter

    See file memory.c on how to compile this code.

    Implement the functions __malloc_impl, __calloc_impl,
    __realloc_impl and __free_impl below. The functions must behave
    like malloc, calloc, realloc and free but your implementation must
    of course not be based on malloc, calloc, realloc and free.

    Use the mmap and munmap system calls to create private anonymous
    memory mappings and hence to get basic access to memory, as the
    kernel provides it. Implement your memory management functions
    based on that "raw" access to user space memory.

    As the mmap and munmap system calls are slow, you have to find a
    way to reduce the number of system calls, by "grouping" them into
    larger blocks of memory accesses. As a matter of course, this needs
    to be done sensibly, i.e. without wasting too much memory.

    You must not use any functions provided by the system besides mmap
    and munmap. If you need memset and memcpy, use the naive
    implementations below. If they are too slow for your purpose,
    rewrite them in order to improve them!

    Catch all errors that may occur for mmap and munmap. In these cases
    make malloc/calloc/realloc/free just fail. Do not print out any 
    debug messages as this might get you into an infinite recursion!

    Your __calloc_impl will probably just call your __malloc_impl, check
    if that allocation worked and then set the fresh allocated memory
    to all zeros. Be aware that calloc comes with two size_t arguments
    and that malloc has only one. The classical multiplication of the two
    size_t arguments of calloc is wrong! Read this to convince yourself:

    https://cert.uni-stuttgart.de/ticker/advisories/calloc.en.html

    In order to allow you to properly refuse to perform the calloc instead
    of allocating too little memory, the __try_size_t_multiply function is
    provided below for your convenience.
    
*/

#include <stddef.h>
#include <sys/mman.h>
#include <stdio.h>
#include <sys/errno.h>
#include <string.h>
#include <unistd.h>

/* Predefined helper functions */

static void *__memset(void *s, int c, size_t n) {
  unsigned char *p;
  size_t i;

  if (n == ((size_t) 0)) return s;
  for (i=(size_t) 0,p=(unsigned char *)s;
       i<=(n-((size_t) 1));
       i++,p++) {
    *p = (unsigned char) c;
  }
  return s;
}

static void *__memcpy(void *dest, const void *src, size_t n) {
  unsigned char *pd;
  const unsigned char *ps;
  size_t i;

  if (n == ((size_t) 0)) return dest;
  for (i=(size_t) 0,pd=(unsigned char *)dest,ps=(const unsigned char *)src;
       i<=(n-((size_t) 1));
       i++,pd++,ps++) {
    *pd = *ps;
  }
  return dest;
}

/* Tries to multiply the two size_t arguments a and b.

   If the product holds on a size_t variable, sets the 
   variable pointed to by c to that product and returns a 
   non-zero value.
   
   Otherwise, does not touch the variable pointed to by c and 
   returns zero.

   This implementation is kind of naive as it uses a division.
   If performance is an issue, try to speed it up by avoiding 
   the division while making sure that it still does the right 
   thing (which is hard to prove).

*/
static int __try_size_t_multiply(size_t *c, size_t a, size_t b) {
  size_t t, r, q;

  /* If any of the arguments a and b is zero, everthing works just fine. */
  if ((a == ((size_t) 0)) ||
      (a == ((size_t) 0))) {
    *c = a * b;
    return 1;
  }

  /* Here, neither a nor b is zero. 

     We perform the multiplication, which may overflow, i.e. present
     some modulo-behavior.

  */
  t = a * b;

  /* Perform Euclidian division on t by a:

     t = a * q + r

     As we are sure that a is non-zero, we are sure
     that we will not divide by zero.

  */
  q = t / a;
  r = t % a;

  /* If the rest r is non-zero, the multiplication overflowed. */
  if (r != ((size_t) 0)) return 0;

  /* Here the rest r is zero, so we are sure that t = a * q.

     If q is different from b, the multiplication overflowed.
     Otherwise we are sure that t = a * b.

  */
  if (q != b) return 0;
  *c = t;
  return 1;
}

/* End of predefined helper functions */

/* Your helper functions 

   You may also put some struct definitions, typedefs and global
   variables here. Typically, the instructor's solution starts with
   defining a certain struct, a typedef and a global variable holding
   the start of a linked list of currently free memory blocks. That 
   list probably needs to be kept ordered by ascending addresses.

*/

// header that will sit at the beginning of each block of memory to provide space for 
// information about the size of the block of memory, a pointer to the memory, and 
// where the next block of free memory is (if in the linked list of free memory)
struct memblock {
    size_t size; // size of block currently
    struct memblock* next; // next free memblock (if this block is in the free list)
    size_t mem_size; // original size of mmap
    void* mem_start; // where the original mmap started - to determine coalescing
};
typedef struct memblock memblock_t;


#define HEADER_SIZE (sizeof(memblock_t)) // save size of header_t for easy use in code
#define WORD_SIZE ((size_t) 4)
#define PAGE_SIZE ((size_t) 4096)
#define MIN_MMAP_SIZE ((size_t) 33554432) // 32 MB minimum for mmap

static memblock_t* free_mem_head = NULL; // global variable for start of free memory linked list

static void __munmap_memblocks() {
  memblock_t* current = free_mem_head;
  memblock_t* prev = NULL;

  while (current) {
    if (current->size == current->mem_size && current->mem_start == (void*) current) {
      memblock_t* next = current->next;
      char* msg = "\tMUNMAPPING!!!!";
      write(2, msg, strlen(msg));
      int status = munmap(current->mem_start, current->mem_size);
      if (status < 0){
        char* msg = "ERROR: Munmap failed.";
        write(2, msg, strlen(msg));
        return;
      }
      if (prev == NULL) {
        free_mem_head = next;
        return;
      }
      else {
        prev->next = next;
        return;
      }
      return;
    }
    prev = current;
    current = current->next;
  }
}

static void __coalesce_memblock(memblock_t* ptr) {
    if (ptr == NULL) return;
    if (ptr->next == NULL) return; // can't coalesce the last block
    int coalesced = 0;
    memblock_t* next = ptr->next;
    if (ptr->mem_start == next->mem_start) {
      if ( ( ((void*)ptr) + ptr->size ) == ((void*)next) ) {
         ptr->next = next->next; 
         ptr->size = ptr->size + next->size;
         coalesced = 1;
         //char* msg = "Coalesced.\n";
         //write(2, msg, strlen(msg));
         next = ptr->next;
         
         if (next == NULL) {
           if (coalesced) __munmap_memblocks();
           return;
         }
         if (next->next == NULL) {
           if (coalesced) __munmap_memblocks();
           return;
         } 
         if (next->mem_start == next->next->mem_start) {
           if ( ( ((void*)next) + next->size ) == ((void*)next->next) ) {
             memblock_t* temp = next->next;
             next->next = temp->next;
             next->size = next->size + temp->size;
             coalesced = 1;
             __munmap_memblocks();
           }
         }
         
         return;
      }
      return;
    }
    return;
    //char* msg = "Did not coalesce.\n";
    //write(2, msg, strlen(msg));
}

static void __insert_memblock(memblock_t* memblock) {
    memblock_t* current = free_mem_head;
    memblock_t* prev = NULL;
    while (current != NULL) {
        // memblock is lower in address value than current, correct spot found
        if (((void*)memblock) < ((void*)current)) {
          // should be added before the head
          if (prev == NULL) {
            memblock->next = free_mem_head;
            free_mem_head = memblock;
            __coalesce_memblock(memblock);
          }
          // insert between prev and current
          prev->next = memblock;
          memblock->next = current;
          __coalesce_memblock(memblock);
          __coalesce_memblock(prev);
          return;
        }
        prev = current;
        current = current->next;
    }
    if (prev) {
      prev->next = memblock;
      __coalesce_memblock(prev);
    }
    free_mem_head = memblock;
    __coalesce_memblock(memblock);
    return;
}

// round sizes to be word aligned (4 bytes) for purposes of allocating memory to user
static size_t __round_size_word(size_t size) {
    size = size + HEADER_SIZE;
    if (size % WORD_SIZE != 0) size += size % WORD_SIZE;
    return size;
}

// round sizes to be page aligned for purposes of mmap'ing new memory
static size_t __round_size_page(size_t size) {
    size = size + HEADER_SIZE;
    if (size % PAGE_SIZE != 0) size += size % PAGE_SIZE;
    return size;
}

// map a new block of memory at least as large as size + header - page aligned to PAGE_SIZE
static void __mmap_memblock(size_t size) {
    size = __round_size_page(size);
    if (size < MIN_MMAP_SIZE) size = MIN_MMAP_SIZE; // minimum size of 32 MB
    void* ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    // error checking mmap
    if (ptr == MAP_FAILED) {
        char* msg = "Error: mmap failed.\n";
        write(2, msg, strlen(msg));
        return;
    }

    // update memblock_t header information for new memory block and add to linked list
    
    ((memblock_t*)ptr)->size = size;
    ((memblock_t*)ptr)->mem_start = ptr;
    ((memblock_t*)ptr)->mem_size = size;
    ((memblock_t*)ptr)->next = NULL;
    __insert_memblock((memblock_t*)ptr);

    return;
}

// search the list for an appropriately sized memblock or make a new one and return it
static memblock_t* __get_memblock(size_t size) {
    memblock_t* current = free_mem_head;
    size = __round_size_word(size);
    while (current) {
        if (current->size >= size) {
            return current;
        }
        current = current->next;
    }

    // no free block large enough, let's make a new one
    __mmap_memblock(size);
    // and check again
    current = free_mem_head;
    while (current) {
        if (current->size >= size) {
            return current;
        }
        current = current->next;
    }

    // we failed, return null
    char* msg = "Error: could not get memblock\n";
    write(2, msg, strlen(msg));
    return NULL;
}

/* End of your helper functions */






/* Start of the actual malloc/calloc/realloc/free functions */

void __free_impl(void *);

void *__malloc_impl(size_t size) {

  // requested to allocate 0 bytes
  if (size == (size_t) 0) return NULL;

  memblock_t* p = __get_memblock(size); // the memblock version
  size_t user_size = __round_size_word(size); // get the size we're going to take from it
  
  if (p) {
      // TODO: check to see if remaining size in memblock after allocation is at least
      //       HEADER_SIZE + WORD_SIZE in length, otherwise allocate the entire block
      //       and remove it from the linked list
      size_t p_size = p->size;
      if (p_size - (user_size + HEADER_SIZE) < __round_size_page(HEADER_SIZE)) {
          char* msg = "We need to remove the block from free list\n";
          write(2, msg, strlen(msg));
      }
      void* user_ptr = ((void*) p) + p_size; // go to the end of block
      user_ptr = user_ptr - (HEADER_SIZE + user_size); // go back the size of header and size from user
      p->size = p_size - (HEADER_SIZE + user_size); // adjust p size accordingly

      // update header info for user memory
      ((memblock_t*)user_ptr)->size = user_size + HEADER_SIZE; 
      ((memblock_t*)user_ptr)->mem_size = p->mem_size;
      ((memblock_t*)user_ptr)->mem_start = p->mem_start;

      // point at where the memory starts and return to user
      user_ptr = user_ptr + HEADER_SIZE;
      return user_ptr;
  } 
  // couldn't allocate memory, return NULL
  char* msg = "Error: malloc failed to allocate\n";
  write(2, msg, strlen(msg));
  errno = ENOMEM;
  return NULL;
}

void *__calloc_impl(size_t nmemb, size_t size) {
  size_t total;
  memblock_t* user_ptr;
  // check for overflow
  if (!__try_size_t_multiply(&total, nmemb, size)) return NULL;

  // no overflow, get memory with malloc and if successful, write 0's across all bytes
  user_ptr = __malloc_impl(total);
  if (user_ptr != NULL) {
    __memset(user_ptr, 0, total);
    return user_ptr;
  }

  // didn't succeed in allocating memory from malloc, set errno and return NULL
  errno = ENOMEM;
  char* msg = "Error: calloc failed to allocate\n";
  write(2, msg, strlen(msg));
  return NULL;  
}

void *__realloc_impl(void *ptr, size_t size) {
  void* dest = NULL;

  // they're using it to malloc
  if (ptr == NULL) return __malloc_impl(size); 

  // they're using it to free
  if (size == 0) {
    __free_impl(ptr);
    return NULL;
  }

  // they're actually resizing
  dest = __malloc_impl(size);
  if (!dest) return NULL;
  // need to free up the old memory and give them the new memory
  memblock_t* return_memblock = (memblock_t*) (ptr - HEADER_SIZE);
  size_t copy_size = return_memblock->size;
  if (size < copy_size) copy_size = size;
  __memcpy(dest, ptr, copy_size);
  __free_impl(ptr);
  return dest;  
}

void __free_impl(void *ptr) {
  if (ptr == NULL) return;
  memblock_t* freed = ((memblock_t*) (ptr - HEADER_SIZE));
  __insert_memblock(freed);
}

/* End of the actual malloc/calloc/realloc/free functions */

