#include <string.h>
#include <stdio.h>
#include <errno.h>

int lines = 10;
int BUFF_SIZE = 4096;

int _strln(const char *str) {
    if (*str)
        return 1 + _strln(++str);
    return 0;
}

int _atoi(const char *str) {
    int res = 0;
    while (*str) {
        res = res * 10 + (*str) - '0';
        ++str;
    }
    return res;
}

int main(int argc, char const *argv[]) {
    if(argc > 3) {
        errno = E2BIG;
        printf("ERROR: %s\n",strerror(errno));
    }

    char* int_test = "123456";
    printf("%d\n", _atoi(int_test));
}