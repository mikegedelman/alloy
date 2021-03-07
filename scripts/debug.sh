#!/bin/bash

qemu-system-i386 -kernel target/x86-alloy/debug/kernel.bin -hda kernel/resources/hello_rust.tar -S -s & \
QEMU_PID=$! && \
gdb target/x86-alloy/debug/kernel.bin  \
        -ex 'target remote localhost:1234' \
        -ex 'layout src' \
        -ex 'break kernel_main' \
        -ex 'continue' && kill $QEMU_PID
