#!/bin/bash

echo "" > stdout
echo "---" >> stdout
 qemu-system-i386 -kernel os-multiboot.bin -serial stdio \
        -netdev user,id=f1,hostfwd=tcp:127.0.0.1:8000-:8000 \
        -device rtl8139,netdev=f1 \
        -object filter-dump,id=ff1,netdev=f1,file=dump.dat \
        -S -s >> stdout & \
QEMU_PID=$! && \
gdb os-multiboot.bin  \
        -ex 'target remote localhost:1234' \
        -ex 'layout src' \
        -ex "break $1" \
        -ex 'continue' && kill $QEMU_PID