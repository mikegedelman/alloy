#!/bin/sh
# You need grub-mkrescue AND grub-pc-bin installed for this script to work.
# Apparently grub-mkrescue is hard to get going on OSX, so I just installed
# them in a docker container to use this script with.
set -e

# make multiboot
mkdir -p isodir
mkdir -p isodir/boot
mkdir -p isodir/boot/grub

cp os-multiboot.bin isodir/boot/ros.bin
cat > isodir/boot/grub/grub.cfg << EOF
GRUB_DEFAULT=0
GRUB_HIDDEN_TIMEOUT=0
GRUB_HIDDEN_TIMEOUT_QUIET=true
GRUB_TIMEOUT=10
GRUB_DISTRIBUTOR=`lsb_release -i -s 2> /dev/null || echo Debian`
GRUB_CMDLINE_LINUX_DEFAULT="quiet splash"
GRUB_CMDLINE_LINUX=""
menuentry "ros" {
	multiboot /boot/ros.bin
}
EOF
docker run -v $(pwd):/work -w /work toolchain grub-mkrescue -o ros.iso isodir