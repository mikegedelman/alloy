#!/bin/sh

scripts/run.sh -S -s > stdout 2> stderr &
QEMU_PID=$!
echo "qemu pid: $QEMU_PID"
gdb os-multiboot.bin  \
        -ex 'target remote localhost:1234' \
        -ex 'add-symbol-file userspace/user.exe' \
        -ex 'layout src' \
        -ex "break $1" \
        -ex 'continue'
kill $QEMU_PID
