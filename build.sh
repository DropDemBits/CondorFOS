#!/bin/bash
set -e
. ./headers.sh

mkdir -p ./sysroot

for PROJECT in $PROJECTS; do
	(cd $PROJECT && DESTDIR="$SYSROOT" $MAKE install)
done
