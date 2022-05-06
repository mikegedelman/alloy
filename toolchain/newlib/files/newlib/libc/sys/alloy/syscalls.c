
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/times.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <stdio.h>


#define SYSCALL 0x80

extern int __alloy_syscall(int syscall_no, int arg1, int arg2, int arg3, int arg4);

enum Syscalls {
	EXIT = 1,
	CLOSE = 2,
	EXECVE = 3,
	FORK = 4,
	FSTAT = 5,
	GETPID = 6,
	ISATTY = 7,
	KILL = 8,
	LINK = 9,
	LSEEK = 10,
	OPEN = 11,
	READ = 12,
	SBRK = 13,
	STAT = 14,
	TIMES = 15,
	UNLINK = 16,
	WAIT = 17,
	WRITE = 18,
	GETTIMEOFDAY = 19
};

void _exit() {
	__asm__ volatile (
      	"movl %0, %%eax\n\t"
      	"int %1"
		:: "i" (0x1),
		   "i" (0x80)
		: "eax", "ebx"
	);
}
int close(int file) {
    return -1;
}
//char **environ; /* pointer to array of char * strings that define the current environment variables */
char *__env[1] = { 0 };
char **environ = __env;

#undef errno
extern int errno;
int execve(char *name, char **argv, char **env) {
    errno = ENOMEM;
    return -1;
}

int fork() {
    errno = EAGAIN;
    return -1;
}

int fstat(int file, struct stat *st) {
    st->st_mode = S_IFCHR;
    return 0;
}

int getpid() {
    return 1;
}

int isatty(int file) {
    return 1;
}

int kill(int pid, int sig) {
    errno = EINVAL;
    return -1;
}

int link(char *old, char *new) {
    errno = EMLINK;
    return -1;
}

int lseek(int file, int ptr, int dir) {
    return 0;
}
int open(const char *name, int flags, ...) {
    return -1;
}

int read(int fd, char *ptr, int len) {
    return __alloy_syscall(READ, fd, (int) ptr, len, 0);
}

int write(int fd, char *ptr, int len) {
    return __alloy_syscall(WRITE, fd, (int) ptr, len, 0);
}

caddr_t sbrk(int incr) {
    extern char _end;		/* Defined by the linker */
    static char *heap_end;
    char *prev_heap_end;

    if (heap_end == 0) {
        heap_end = &_end;
    }
    prev_heap_end = heap_end;
//    if (heap_end + incr > stack_ptr) {
//        write (1, "Heap and stack collision\n", 25);
//        abort ();
//    }

    heap_end += incr;
    return (caddr_t) prev_heap_end;
}
int stat(const char *file, struct stat *st) {
    st->st_mode = S_IFCHR;
    return 0;
}
clock_t times(struct tms *buf) {
    return -1;
}

int unlink(char *name) {
    errno = ENOENT;
    return -1;
}

int wait(int *status) {
    errno = ECHILD;
    return -1;
}

extern int write(int fd, char *ptr, int len);
int gettimeofday(struct timeval * restrict,  void * restrict);
