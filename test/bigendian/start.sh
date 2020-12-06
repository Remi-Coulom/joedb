#!/bin/bash
qemu-system-ppc -bios ./openbios-ppc -hda debian-wheezy-powerpc.qcow2 -L bios -m 2047M -g 1024x768x8 -device e1000,netdev=net0 -netdev user,id=net0,hostfwd=tcp::2222-:22
