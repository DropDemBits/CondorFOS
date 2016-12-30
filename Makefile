build_kernel:
	make -C ./kernel/ -f ./Makefile iso
build_usrlnd:
	
build_libc:
	
clean:
	make -C ./kernel/ -f ./Makefile clean
	
build: build_kernel build_usrlnd build_libc
	
run-bochs: build
	bochs -q -rc skip -f bochsrc
run-qemu: build
	qemu-system-i386 -cdrom condor.iso

