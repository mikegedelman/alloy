#include <stdint.h>
#include <stddef.h>
#include <kernel/string.h>

void *memset(void *dest, int c_int, size_t n) {
    uint8_t c = c_int;
    uint8_t *buf = dest;
    for (int i = 0; i < n; i++) {
        buf[i] = c;
    }
    return dest;
}

void *memcpy(void *dest, const void *src, size_t n) {
    size_t i;

    for (i = 0; i < n; ++i)
        ((unsigned char *) dest)[i] = ((unsigned char *) src)[i];

    return dest;
}

int memcmp(uint8_t *a, const uint8_t *b, size_t n) {
    for (int i = 0; i < n; i++) {
        if (*a < *b) {
            return -1;
        } else if (*a > *b) {
            return 1;
        }

        a++;
        b++;
    }
    return 0;
}

bool strcmp(const char *a, const char *b) {
    size_t cur_pos = 0;
    while (a[cur_pos] != 0 && b[cur_pos] != 0) {
        if (a[cur_pos] != b[cur_pos]) {
            return false;
        }
        cur_pos += 1;
    }

    return a[cur_pos] == b[cur_pos];
}

size_t strlen(const char *c) {
    size_t count = 0;
    while (*c != 0) {
        count++;
        c++;
    }
    return count;
}