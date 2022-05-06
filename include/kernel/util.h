#ifndef _KERNEL_UTIL_H
#define _KERNEL_UTIL_H

void exit_qemu();
void panic(const char *msg);

#define assert(expr, msg) if (!(expr)) { panic(msg); }

#endif
