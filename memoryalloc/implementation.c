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
    size_t size;
    struct memblock* next;
    void* mem_start;
};
typedef struct memblock memblock_t;


#define HEADER_SIZE (sizeof(memblock_t)) // save size of header_t for easy use in code
#define WORD_SIZE ((size_t) 4)
#define PAGE_SIZE ((size_t) 4096)
#define MIN_MMAP_SIZE ((size_t) 33554432) // 32 MB minimum for mmap

static memblock_t* free_mem_head = NULL; // global variable for start of free memory linked list

void __insert_memblock(memblock_t* memblock) {
    if (free_mem_head == NULL) {
      free_mem_head = memblock;
      return;
    }
    memblock_t* current = free_mem_head;
    memblock_t* prev = NULL;
    while (current) {
        // memblock is lower in address value than current, correct spot found
        if (memblock < current) {
          prev->next = memblock;
          memblock->next = current;
          return;
        }
        prev = current;
        current = current->next;
    }
    // hit the end of the list
    prev->next = memblock;
    return;
}

// round sizes to be word aligned (4 bytes) for purposes of allocating memory to user
size_t __round_size_word(size_t size) {
    size = size + HEADER_SIZE;
    if (size % WORD_SIZE != 0) size += size % WORD_SIZE;
    return size;
}

// round sizes to be page aligned for purposes of mmap'ing new memory
size_t __round_size_page(size_t size) {
    size = size + HEADER_SIZE;
    if (size % PAGE_SIZE != 0) size += size % PAGE_SIZE;
    return size;
}

// map a new block of memory at least as large as size + header - page aligned to PAGE_SIZE
memblock_t* __mmap_memblock(size_t size) {
    size = __round_size_page(size);
    if (size < MIN_MMAP_SIZE) size = MIN_MMAP_SIZE; // minimum size of 32 MB
    memblock_t* new_memblock = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, 0, 0);
    // error checking mmap
    if (new_memblock == MAP_FAILED) {
        char* msg = "Error: mmap failed.\n";
        write(2, msg, strlen(msg));
        return NULL;
    }
    // update memblock_t header information for new memory block and add to linked list
    new_memblock->size = size;
    __insert_memblock(new_memblock);

    return new_memblock;
}

// search the list for an appropriately sized memblock or make a new one and return it
memblock_t* __get_memblock(size_t size) {
    memblock_t* current = free_mem_head;
    size = __round_size_word(size);
    while (current) {
        if (current->size >= size) {
            return current;
        }
        current = current->next;
    }

    // no free block large enough, let's make a new one
    //printf("Trying to map a new block of size %lu", size);
    return __mmap_memblock(size);
}

/* End of your helper functions */






/* Start of the actual malloc/calloc/realloc/free functions */

void __free_impl(void *);

void *__malloc_impl(size_t size) {

  // requested to allocate 0 bytes
  if (size == (size_t) 0) return NULL;

  memblock_t* p = __get_memblock(size);
  size_t round_size = __round_size_word(size);
  if (p) {
      // TODO: check to see if remaining size in memblock after allocation is at least
      //       HEADER_SIZE + WORD_SIZE in length, otherwise allocate the entire block
      //       and remove it from the linked list
      void* user_ptr = (void*)p + p->size; // go to the end
      user_ptr -= round_size; // go back the rounded size + header size
      ((memblock_t*)user_ptr)->size = round_size;
      ((memblock_t*)user_ptr)->next = NULL;
      p->size -= round_size; // update original memblock's size
      user_ptr = ((memblock_t*)user_ptr)->mem_start;
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
  if (user_ptr) {
    __memset(user_ptr, 0, total);
  }

  // didn't succeed in allocating memory from malloc, set errno and return NULL
  errno = ENOMEM;
  perror("Error: calloc failed to allocate");
  return NULL;  
}

void *__realloc_impl(void *ptr, size_t size) {
  void* dest = NULL;
  void* src = NULL;
  __memcpy(dest, src, 0);
  return NULL;  
}

void __free_impl(void *ptr) {
  if (ptr == NULL) return;
  __insert_memblock(ptr - HEADER_SIZE);
}

/* End of the actual malloc/calloc/realloc/free functions */

