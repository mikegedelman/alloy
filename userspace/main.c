#include <stdio.h>

void main() {
    int fd = open("alloy.txt", "r");
    if (fd < 0) {
        printf("Unable to open alloy.txt\n");
        exit(-1);
    }
}
