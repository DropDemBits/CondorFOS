TARGET_ARCH=i386

.PHONY: clean build run-bochs run-qemu

all: build

clean:
	./clean.sh
build:
	./build.sh
iso:
	./iso.sh

run-bochs: iso
	bochs -q -rc skip -f bochsrc
run-qemu: iso
	qemu-system-i386 -cdrom condor.iso
