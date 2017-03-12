#!/bin/bash
set -e
. ./build.sh

mkdir -p ./isodir/boot/grub
cat > ./isodir/boot/grub/grub.cfg << EOF
menuentry "CondorFOS" {
	multiboot /boot/condor.kern
	module /boot/programs/test
}
EOF

cp ./kernel/condor.kern ./isodir/boot/condor.kern

# Copy programs
cp -Ru userland/programs/bin/ isodir/boot/programs/

grub-mkrescue -o condor.iso ./isodir
