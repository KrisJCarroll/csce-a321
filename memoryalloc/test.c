#include <stdio.h>
#include <stdlib.h>

int main(int argc, const char *argv[]) {
    char* essay = malloc(sizeof(char)*64000);
    char* bigger_essay = malloc(sizeof(char) * 256000);
    char* little_essay = malloc(sizeof(char) * 32000);
    free(little_essay);
    free(bigger_essay);
    free(essay);
}