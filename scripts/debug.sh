#!/bin/bash

KERNEL="../target/x86-alloy/debug/boot"
HDA="-hda kernel/resources/hello_rust.tar"
qemu-system-i386 -kernel $KERNEL -S -s & \
QEMU_PID=$! && \
gdb $KERNEL  \
        -ex 'target remote localhost:1234' \
        -ex 'layout asm' \
        -ex 'break *0x00100010' \
        -ex 'continue' && kill $QEMU_PID
