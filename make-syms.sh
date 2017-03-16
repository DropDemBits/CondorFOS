#!/bin/bash
set -e
. ./iso.sh

i686-elf-objcopy --only-keep-debug ./kernel/condor.kern ./kernel/kern.sym
nm kernel/condor.kern | grep " T " | awk '{print $1" "$3}' > condor.sym
i686-elf-objcopy --strip-debug ./kernel/condor.kern
