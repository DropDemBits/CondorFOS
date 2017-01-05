TARGET_ARCH=i386

.PHONY: clean build run-bochs run-qemu

all: build

clean:
	make -C ./kernel/ -f ./Makefile clean
	make -C ./libc/ -f ./Makefile clean
	rm -rf ./isodir ./sysroot

build:

install:
	make -C ./libc/ -f ./Makefile install-headers
	make -C ./kernel/ -f ./Makefile install-headers
	make -C ./libc/ -f ./Makefile install-binaries
	make -C ./kernel/ -f ./Makefile install-binaries
	bash prep_iso.sh
	make -C ./kernel/ -f ./Makefile iso
run-bochs: install
	bochs -q -rc skip -f bochsrc
run-qemu: install
	qemu-system-i386 -cdrom condor.iso
