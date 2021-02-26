#!/bin/sh
# You need grub-mkrescue AND grub-pc-bin installed for this script to work.
# Apparently grub-mkrescue is hard to get going on OSX, so I just installed
# them in a docker container to use this script with.
set -e

make multiboot
mkdir -p isodir
mkdir -p isodir/boot
mkdir -p isodir/boot/grub

cp os-multiboot.bin isodir/boot/ros.bin
cat > isodir/boot/grub/grub.cfg << EOF
menuentry "ros" {
	multiboot /boot/ros.bin
}
EOF
grub-mkrescue -o ros.iso isodir