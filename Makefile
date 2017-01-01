.PHONY: clean build run-bochs run-qemu

all: build

clean:
	make -C ./kernel/ -f ./Makefile clean
	rm -rf ./isodir

build:
	bash prep_iso.sh
	make -C ./kernel/ -f ./Makefile iso
run-bochs: build
	bochs -q -rc skip -f bochsrc
run-qemu: build
	qemu-system-i386 -cdrom condor.iso
