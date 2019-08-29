#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

/* TODO: Change to use memmove
         Also rewrite as a linked list.        
*/

typedef struct {
    int *data;          // The array of data for FIFO queue
    size_t length;      // The used part of the array
    size_t size;        // Maximum size of the array as allocated
} fifo_t;


/* Different way to do above

struct __fifo_struct_t {
    int *data;
    size_t length;
    size_t size;
};
typedef struct __fifo_struct_t fifo_t; 

*/

void fifo_init(fifo_t *fifo) {
    fifo->data = NULL;
    fifo->length = 0;
    fifo->size = 0;
}

void fifo_clear(fifo_t *fifo) {
    if (fifo->data != NULL) {
        free(fifo->data);
        fifo->data = NULL;
    }
    fifo->length = 0;
    fifo->size = 0;
}

static void __fifo_alloc_data(fifo_t *fifo, size_t n) {
    void *temp;
    
    if (n == ( (size_t) 0 ) )
        return;
    if (fifo->data == NULL) {
        // fifo->data = malloc(n * sizeof(int)); don't use for array allocation - don't multiply in malloc
        fifo->data = calloc(n, sizeof(int));
        
        // Check to make sure it worked and die if it didn't
        if (fifo->data == NULL) {
            fprintf(stderr, "Not enough memory: %s\n", strerror(errno));
            exit(1);
        }

        fifo->size = n;
    } else {
        if (n <= fifo->size) return;
        temp = realloc(fifo->data, n * sizeof(int)); // This could overflow
        if (temp == NULL) {
            fprintf(stderr, "Not enough memory: %s\n", strerror(errno));
            exit(1);
        }
        fifo->data = (int *) temp;
        fifo->size = n;
    }
 }

void fifo_queue_in(fifo_t *fifo, int in[], size_t n) {
    size_t i;

    __fifo_alloc_data(fifo, fifo->length + n);
    for (i = 0; i < n; i++) {
        fifo->data[i + fifo->length] = in[i];
    }
    fifo->length += n;
}

size_t fifo_queue_out(fifo_t *fifo, int out[], size_t n) {
    size_t m, i;

    m = n;
    if (fifo->length < m)
        m = fifo->length;

    for (i = 0; i < m; i++) {
        out[i] = fifo->data[i];
    }
    for (i = m; i < fifo->length; i++) {
        fifo->data[i - m] = fifo->data[i];
    }
    fifo->length -= m;

    return m;
}

int main(int argc, char **argv) {
    fifo_t fifo;
    int in[16], out[16];
    int n, m;

    fifo_init(&fifo);
    while (1) {
        printf("Type in the number of new elements: ");
        scanf("%d", &n);

        printf("Type in %d new elements\n", n);
        for(int i = 0; i < n; i++) {
            scanf("%d", &in[i]);
        }
        printf("Queueing in the elements\n");
        fifo_queue_in(&fifo, in, (size_t) n);

        printf("Type in the number of elements to queue out: ");
        scanf("%d", &n);
        printf("Queueing out &d elements...", n);
        m = fifo_queue_out(&fifo, out, (size_t) n);
        printf("I could queue out &d elements:\n", m);

        for (int i = 0; i < m; i++) {
            printf("%d\n", out[i]);
        }
    }


    return 0;
}