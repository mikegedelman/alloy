#include <stdio.h>
#include <unistd.h>
#include <string.h>

void main() {
    puts("Proc2 started.");
    const char *hello = "Hello from another process";
    write(0, hello, strlen(hello));
}
