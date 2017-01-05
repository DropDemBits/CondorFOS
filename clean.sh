#!/bin/bash
set -e
. ./vars.sh

for PROJECT in $PROJECTS; do
	(cd $PROJECT && DESTDIR="$SYSROOT" $MAKE clean)
done

rm -rf sysroot
rm -rf isodir
