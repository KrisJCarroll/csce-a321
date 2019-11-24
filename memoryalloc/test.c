#include <stdio.h>
#include <stdlib.h>

int main(int argc, const char *argv[]) {
    char* essay = malloc(sizeof(char)*64000);
    char* bigger_essay = malloc(sizeof(char) * 256000);
    char* little_essay = malloc(sizeof(char) * 32000);
    free(little_essay);
    free(bigger_essay);
    free(essay);

    char* jiggabyte = malloc(1024*1024*1024);
    free(jiggabyte);

    printf("Getting ready to test calloc...");
    char* all_zeroes = calloc(10, sizeof(char));
    printf("Here's all_zeroes: %s",all_zeroes);
    printf("%s", "\n");
    
    char* count_to_nine = calloc(10, sizeof(char));
    for(int i = 0; i < 10; i++) {
        count_to_nine[i] = i + '0';
    }
    printf("Here's count_to_nine after we changed it: %s", count_to_nine);
    printf("%s", "\n");

    free(all_zeroes);
    free(count_to_nine);

}