void __free_impl(void *ptr);
    // munmap needs a length, how to get it?
/* add a header to the beginning of each block of memory mmap'ed to store information like size and such
   so we can edit and free appropriately when we need to

*/

typedef struct {
    size_t length;
    void* mmap_start;
    size_t mmap_size;
    void* next;
} header_t;

// then when user passes pointer, subtract sizeof(header_t) and you're at the beginning of the header and can
// access all of this information

header_t* free_memory = NULL;

// when looking at blocks of memory, look at pointer to header + length of block/header and 
// the pointer signified by void* next - if these are the same place, we need to merge the two blocks into
// one large block

    // basically squish adjacent free blocks together when recognized

// if entire area of memory allotted from mmap is free, it's safe to munmap

// most of these functions need to check to see if they've ever been initialized in some way before
// if they have, do nothing, otherwise set up the necessary data structures

// will not be told when the last free() will be called or the last malloc/calloc etc. so need to 
// identify cases where munmap should be used and use it immediately

// when partially using a free block, move the header past the point of the memory being allocated and
// update the length appropriately
  // if there is not enough space for the header in the remaining free area of memory, allocate the extra to the user

/* Steps for assignment in order of priority:
    
    1) typdef header

    2) implement free
        a) subtract header size
        b) insert block into linked list
        c) coalesce blocks around newly inserted block
        d) possibly munmap block if it fills a mmap'ed block completely (STUB)

    3) implement malloc
        a) get block from linked list
            i) if size == 0 - does a weird, special thing that can be recognized later
        b) if unsuccessful, mmap new block in
            i) try again
        c) add header size to pointer

    4) implement calloc
        a) use malloc and write zeroes

    5) implement realloc(old_ptr, new_size) - *** SENSITIVE SUB CASES HERE ***
        - if old_ptr is NULL, behaves like malloc
        - if new_size is 0, behaves like free
        - otherwise old_ptr -> subtracting header size -> find old_size
        - if old_size is larger than new_size
            * make a header for the chunk of memory no longer needed and insert into the linked list of free blocks
        - old_size < new_size
            - basic
                - malloc the new_size
                - copy contents of old block w/o header
                - free old pointers
            - optimized
                - before the copy check in list of free memory if existing block in list can satisfy the request

    6) Test
    7) Test again

