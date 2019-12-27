#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char **argv) {
    int fd1, fd2;
    char str1[] = "naughty";
    char str2[] = "nice";
    char buf1[42];
    char buf2[42];

    fd1 = open("file.txt", O_CREAT | O_RDWR, 0644);
    write(fd1, str1, sizeof(str1));
    lseek(fd1, 0, SEEK_SET);

    unlink("file.txt");

    fd2 = open("file.txt", O_CREAT | O_RDWR, 0644);
    write(fd2, str2, sizeof(str2));

    memset(buf1, '\0', sizeof(buf1));
    read(fd1, buf1, sizeof(buf1));

    close(fd1);

    fd1 = open("file.txt", O_CREAT | O_RDWR, 0644);

    unlink("file.txt");

    memset(buf2, '\0', sizeof(buf2));
    read(fd1, buf2, sizeof(buf2));

    close(fd2);
    close(fd1);

    printf("%s or %s\n", buf1, buf2);

    return 0;
}