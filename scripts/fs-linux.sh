#!ybin/bash

dd if=/dev/zero of=hdd.img bs=512 count=204800
DEV=$(sudo losetup -Pf hdd.img --show)
sudo cfdisk $DEV # todo script this out
sudo mkfs.fat -F 16 "${DEV}p1"
# sudo xxd -a $DEV  # check to see if this looks right
sudo mkdir /media/fat
sudo echo "${DEV}p1   /media/fat      msdos   user,noauto,loop,sync     0       0" >> /etc/fstab
mount /media/fat
echo "hello" > /media/fat/hi.txt
# sudo xxd -a $DEV # check again

mkdir resources
dd if=/dev/urandom of=resources/random.bin bs=512 count=8
cp resources/random.bin /media/fat/random.bin
