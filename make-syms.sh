#!/bin/bash
set -e
. ./iso.sh

objcopy --only-keep-debug ./kernel/condor.kern ./kernel/kern.sym
nm kernel/condor.kern | grep " T " | awk '{print $1" "$3}' > condor.sym
objcopy --strip-debug ./kernel/condor.kern
