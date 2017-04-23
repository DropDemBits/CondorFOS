#!/bin/bash
set -e
. ./make-syms.sh

qemu-system-$TARGET_ARCH -s -S -cdrom condor.iso -drive format=raw,file=disk.img
