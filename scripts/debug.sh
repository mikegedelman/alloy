#!/bin/bash

qemu-system-i386 -kernel os-multiboot.bin -hda hdd.img -serial stdio -S -s & \
QEMU_PID=$! && \
gdb os-multiboot.bin  \
        -ex 'target remote localhost:1234' \
        -ex 'layout src' \
        -ex "break $1" \
        -ex 'continue' && kill $QEMU_PID