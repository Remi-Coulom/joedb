#!/bin/bash
qemu-system-ppc -bios ./openbios-ppc -hda debian-wheezy-powerpc.qcow2 -L bios -m 1G -g 1024x768x8 -redir tcp:2222::22
