#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, const char *argv[]) {
    char* null = malloc(0);
    free(null);
    null = calloc(0, 0);
    free(null);
    null = calloc(0, sizeof(int));
    free(null);
    null = calloc(100000, 0);
    free(null);

    char* realbig = malloc(1073741824);
    free (realbig);

    char* too_big = malloc(9223372036854775807);
    free(too_big);

    char* too_big = calloc(9223372036854775807, sizeof(char));
    free(too_big);

    char* new_ptr = realloc(NULL, 1024);
    new_ptr = realloc(new_ptr, 1073741824);
    new_ptr = realloc(new_ptr, 0);
}