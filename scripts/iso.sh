mkdir -p isodir/boot/grub
cp os-multiboot.bin isodir/boot/alloy.bin
cp grub.cfg isodir/boot/grub/grub.cfg
grub-mkrescue -o alloy.iso isodir
