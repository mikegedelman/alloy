#ifdef TEST
#include <kernel/test.h>
#include <kernel/drivers/uart16550.h>
#include <kernel/mem/physical.h>

#define print(msg) serial_write(&com1, msg)
#define assert(test, message) assert(((void)(message), test)) \
 if (test) {  }

void test_first_free_bit() {
    assert
}

void run_tests() {
    serial_write(&com1, "Running tests...");

    test_first_free_bit();
}
#endif