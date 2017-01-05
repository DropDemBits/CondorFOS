#!/bin/bash
set -e
. ./iso.sh

bochs -q -rc skip -f bochsrc
