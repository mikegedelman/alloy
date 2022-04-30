#!/bin/sh
qemu-system-i386 -kernel os-multiboot.bin \
	-serial stdio \
	-hda hdd.img \
	-netdev user,id=f1,hostfwd=tcp:127.0.0.1:8000-:8000 \
	-device rtl8139,netdev=f1 \
	-object filter-dump,id=ff1,netdev=f1,file=dump.dat \
	-device isa-debug-exit,iobase=0xf4,iosize=0x04 \
	-display none $@ \
	--no-reboot
