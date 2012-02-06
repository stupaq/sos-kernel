#!/bin/bash

./tools/make_initrd user/init

cp -f boot/floppy.img floppy.img

mount floppy.img

cp kernel/kernel.bin /mnt/boot/kernel
cp initrd.img /mnt/boot/initrd

umount /mnt

