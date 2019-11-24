#include <stdio.h>
#include <stdlib.h>

int main(int argc, const char *argv[]) {
    char* null = malloc(0);
    free(null);
    char* null = calloc(0, 0);
    free(null);
    char* null = calloc(0, sizeof(int));
    free(null);
    char* null = calloc(100000, 0);
    free(null);

    char* realbig = malloc(1073741824);
    free (realbig);

    char* too_big = malloc(1073741824 * 5);
    free(too_big);

}