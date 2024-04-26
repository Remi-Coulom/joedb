#!/bin/bash
dd if=/dev/zero of=small_disk.fs bs=256k count=1
mkfs.ext4 small_disk.fs
mkdir -p small_disk
sudo mount -o loop=/dev/loop0 ./small_disk.fs ./small_disk
sudo chown $USER ./small_disk
df ./small_disk
