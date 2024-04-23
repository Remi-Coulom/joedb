#!/bin/bash
set -e
dd if=/dev/zero of=small_disk.fs bs=256k count=1
mkfs.ext4 small_disk.fs
mount_point=./test.small_disk
mkdir -p $mount_point
sudo mount -o loop=/dev/loop0 ./small_disk.fs $mount_point
sudo chown $USER $mount_point
df $mount_point
