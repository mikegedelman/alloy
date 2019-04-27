#include <stdint.h>
#include <kernel/term.h>
#include <kernel/stdio.h>
#include <kernel/fs.h>

void kernel_main() {
    term_init();

    puts("kernel ready");

    // char buf[515];
    // chs_read(buf);

    //char* buf = (char*)0x5000;
    char buf[512];
    chs_read(0, 0, 4, 1, buf);

    puts(buf);

    puts("\n");
    print_hex_buf(buf, 10);
     //term_puts(buf);
}
