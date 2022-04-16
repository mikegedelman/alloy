#!/bin/sh

scripts/run.sh -S -s >> stdout & \
QEMU_PID=$! && \
gdb os-multiboot.bin  \
        -ex 'target remote localhost:1234' \
        -ex 'layout src' \
        -ex "break $1" \
        -ex 'continue' && kill $QEMU_PID
