#!/bin/bash
# Linux-specific script to create a FAT16 loopback

dd if=/dev/zero of=hdd.img bs=512 count=204800
DEV=$(hdiutil attach -nomount hdd.img)
diskutil partitionDisk $DEV MBR "MS-DOS FAT16" VOLUME 100M
hdiutil attach -imagekey diskimage-class=CRawDiskImage -nomount hdd.img
# DEV=$(sudo losetup -Pf hdd.img --show)
# sudo cfdisk $DEV # todo script this out
# sudo mkfs.fat -F 16 "${DEV}p1"
# sudo xxd -a $DEV  # check to see if this looks right
# sudo mkdir /media/fat
# sudo echo "${DEV}p1   /media/fat      msdos   user,noauto     0       0" >> /etc/fstab
# mount /media/fat
# echo "hello" > /media/fat/hi.txt
# sudo xxd -a $DEV # check again

dd if=/dev/urandom of=resources/random.bin bs=512 count=8
cp resources/random.bin /Volumes/VOLUME/random.bin
md5 resources/random.bin # 0e2e3d535387642e97880622628d763a
# update md5sum in test.rs if you have to regenerate this
