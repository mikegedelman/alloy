#include <stddef.h>
#include <kernel/string.h>

void *memcpy(void *dest, const void *src, size_t n) {
    size_t i;

    for (i = 0; i < n; ++i)
        ((unsigned char *) dest)[i] = ((unsigned char *) src)[i];

    return dest;
}

bool strcmp(char *a, char *b) {
    size_t cur_pos = 0;
    while (a[cur_pos] != 0 && b[cur_pos] != 0) {
        if (a[cur_pos] != b[cur_pos]) {
            return false;
        }
        cur_pos += 1;
    }

    return a[cur_pos] == b[cur_pos];
}
