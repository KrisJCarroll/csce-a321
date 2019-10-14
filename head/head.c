#include <string.h>
#include <stdio.h>

int lines = 10;
int BUFF_SIZE = 4096;

int _strln(const char *str) {
    if (*str)
        return 1 + _strln(++str);
    return 0;
}

int main(int argc, char const *argv[]) {
    char *msg = "apple";
    printf("%d\n", _strln(msg));
    return 0;
}