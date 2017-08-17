#!/bin/bash
set -e
. ./vars.sh

mkdir -p ./sysroot/usr/local/include/

for PROJECT in $PROJECTS; do
	(cd $PROJECT && DESTDIR="$SYSROOT" make install-headers)
done
