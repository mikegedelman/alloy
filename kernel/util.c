#include <kernel/all.h>

inline void exit_qemu() {
    outl(0xf4, 0x10);
}

void panic(const char *msg) {
    serial_write(&com1, "PANIC: ");
    serial_write(&com1, msg);
    serial_write(&com1, "\n");
    exit_qemu();
}
