#include <string.h>

int lines = 10;
int BUFF_SIZE = 4096;

size_t _strln(const char *str) {
    size_t i;
    for (i = 0; (*str); i++) {}
    return i;
}

int main(int argc, char const *argv[]) {
    char *msg = "apple";
    printf(_strln(msg));
}