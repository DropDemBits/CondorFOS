#!/bin/bash

mkdir -p ./obj
mkdir -p ./bin
i686-elf-gcc -c -o ./obj/test test.S
objcopy --only-section=.text --output-target binary ./obj/test ./bin/test
