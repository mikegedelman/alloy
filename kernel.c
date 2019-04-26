#include <kernel/term.h>
#include <stdint.h>
void kernel_main() {
     term_init();
     term_puts("Kernel ready");
}
