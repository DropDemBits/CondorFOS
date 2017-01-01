#!/bin/bash
mkdir ./isodir/
mkdir ./isodir/boot
mkdir ./isodir/boot/grub/
cat > isodir/boot/grub/grub.cfg << EOF
menuentry "CondorFOS" {
    multiboot /boot/condor.kern
}
EOF
