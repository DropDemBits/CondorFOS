#!/bin/bash
set -e
. ./headers.sh

mkdir -p ./sysroot
mkdir -p ./sysroot/usr/include
mkdir -p ./sysroot/usr/local/include

for PROJECT in $PROJECTS; do
	(cd $PROJECT && DESTDIR="$SYSROOT" $MAKE install)
done
