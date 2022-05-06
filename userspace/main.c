#include <stdio.h>
#include <unistd.h>

void main() {
    puts("Hello from userspace\n");
    char buf[256];
    read(0, buf, 256);
    printf("Read buf: %s\n");
}
