#!/bin/sh
# You need grub-mkrescue AND grub-pc-bin installed for this script to work.
# Apparently grub-mkrescue is hard to get going on OSX, so I just installed
# them in a docker container to use this script with.
set -e

mkdir -p isodir/boot/grub

cp $1 isodir/boot/alloy.bin
# Despite many attempts, GRUB wants to wait for a keypress before booting my kernel.
# A custom bootloader is on the roadmap; just stick with GRUB if we need an ISO for now.
cat > isodir/boot/grub/grub.cfg << EOF
GRUB_DEFAULT=0
GRUB_HIDDEN_TIMEOUT=0
GRUB_HIDDEN_TIMEOUT_QUIET=true
GRUB_TIMEOUT=0
GRUB_RECORDFAIL_TIMEOUT=$GRUB_TIMEOUT
GRUB_DISABLE_OS_PROBER=true
menuentry "alloy" {
	multiboot /boot/alloy.bin
}
EOF
docker run -v $(pwd):/work -w /work mikegedelman/alloy:master grub-mkrescue -o alloy.iso isodir
