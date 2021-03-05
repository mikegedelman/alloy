#!/bin/bash
# Adapted from: https://os.phil-opp.com/testing/

qemu-system-i386 -kernel $1 \
		-device isa-debug-exit,iobase=0xf4,iosize=0x04 \
		-serial stdio \
		-display none

[ $? -eq 33 ] && exit 0 || exit 1