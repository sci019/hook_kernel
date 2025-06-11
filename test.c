#include <stdio.h>
#include <stdlib.h>

int main() {
    int size = 4096 * 4;
    int size2 = 4096 * 8;
    int *buf = malloc(size);
    int *buf2 = malloc(size2);
    printf("malloc: buf = %p \n", buf);
    printf("malloc: buf2 = %p \n", buf2);

    int unexpected_count = 0;

    for (int i = 0; i < size / sizeof(int); i++) {
        *(buf + i) = i;
    }
    unexpected_count = 0;
    for (int i = 0; i < size / sizeof(int); i++) {
        if (*(buf + i) != i) {
            unexpected_count += 1;
        }
    }
    printf("buf: unexpected_count = %d \n", unexpected_count);

    for (int i = 0; i < size2 / sizeof(int); i++) {
        *(buf2 + i) = i;
    }
    unexpected_count = 0;
    for (int i = 0; i < size2 / sizeof(int); i++) {
        if (*(buf2 + i) != i) {
            unexpected_count += 1;
        }
    }
    printf("buf2: unexpected_count = %d \n", unexpected_count);
}