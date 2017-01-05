#!/bin/bash
set -e
. ./vars.sh

mkdir -p ./sysroot/usr/local/include/

cp ./grub/include/multiboot.h $SYSROOT/usr/local/include/multiboot.h
for PROJECT in $PROJECTS; do
	(cd $PROJECT && DESTDIR="$SYSROOT" make install-headers)
done
