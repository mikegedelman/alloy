//#include <fcntl.h>

extern void exit(int code);
extern int main ();

void _start() {
    _init_signal();
    int ex = main();
    exit(ex);
}
