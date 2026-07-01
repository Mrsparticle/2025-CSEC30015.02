#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

void showflag() { printf("flag{race_condition_succeed!}\n"); }

void vuln(char* file) {
    char buf[256];

    int number;
    int index = 0;

    int fd = open(file, O_RDONLY);
    if (fd == -1) {
        perror("open file failed!!");
        return;
    }
    while (1) {
        number = read(fd, buf + index, 128);
        if (number <= 0) {
            break;
        }
        index += number;
    }
}

void check(char* file) {
    struct stat tmp;
    stat(file, &tmp);
    if (tmp.st_size > 255) {
        puts("file size is too large!!");
        exit(0);
    }
}

int main(int argc, char* argv[argc]) {

    if (argc == 2) {
        check(argv[1]);
        vuln(argv[1]);
    }
    else if (argc == 3) {
        if (strcmp(argv[2], "show_info") == 0) {
            void (*showflag_ptr)() = showflag;
            printf("The address of the showflag function: %p\n", *(&showflag_ptr));
            check(argv[1]);
            vuln(argv[1]);
        }
        else {
            puts("Usage ./prog <filename> show_info or ./prog <filename>");
        }
    }
    else {
        puts("Usage ./prog <filename> show_info or ./prog <filename>");
    }
    return 0;
}