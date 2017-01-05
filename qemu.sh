#!/bin/bash
set -e
. ./iso.sh

qemu-system-$TARGET_ARCH -cdrom condor.iso
