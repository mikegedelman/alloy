#!/bin/bash

if [[ $OSTYPE == 'darwin'* ]]; then
	dd if=/dev/zero of=hdd.img bs=512 count=204800
	DEV=$(hdiutil attach -nomount hdd.img)
	diskutil partitiondisk $DEV 1 MBR "MS-DOS FAT16" "NONAME" 0B
	diskutil unmountDisk $DEV
	sudo newfs_msdos -F 16 "${DEV}s1"
	diskutil mount "${DEV}s1"
	echo "hdd.img mounted at ${DEV}s1"
elif [[ $OSTYPE == 'linux'* ]]; then
	dd if=/dev/zero of=hdd.img bs=512 count=204800
	DEV=$(sudo losetup -Pf hdd.img --show)
	sudo cfdisk $DEV # todo script this out
	sudo mkfs.fat -F 16 "${DEV}p1"
	# sudo xxd -a $DEV  # check to see if this looks right
	sudo mkdir /media/fat
	sudo echo "${DEV}p1   /media/fat      msdos   user,noauto,loop,sync     0       0" >> /etc/fstab
	mount /media/fat
fi


