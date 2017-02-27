#!/bin/bash
set -e
. ./make-syms.sh

bochs -q -rc skip -f bochsrc
