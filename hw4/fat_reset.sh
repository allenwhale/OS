#!/bin/sh
umount /dev/loop0
losetup -d /dev/loop0
rm -f myimage.img
rm -rf mountpoint
dd if=/dev/zero of=myimage.img bs=1M count=512
mkfs.vfat -F 32 myimage.img
losetup -o 0 --sizelimit $((512*1024*1024)) /dev/loop0 myimage.img
losetup -a
mkdir -p mountpoint
mount /dev/loop0 mountpoint
mount | grep '/dev/loop'


