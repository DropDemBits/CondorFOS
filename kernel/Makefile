HOST_ARCH=i386

CC=i686-elf-gcc
CFLAGS=-std=gnu99 -ffreestanding -O2 -Wall -Wextra
LDFLAGS=-ffreestanding -O2 -nostdlib -lgcc

OBJS=
ISO=condor.iso
KERN=condor.kern

include arch/$(HOST_ARCH)/make.config

%.o: %.S
	$(CC) -c -o $@ $<
%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

$(KERN): $(OBJS)
	$(CC) -T arch/i386/linker.ld -o $@ $(LDFLAGS) $<
iso: condor.kern
	cp $(KERN) ../isodir/boot/$(KERN)
	grub-mkrescue -o ../$(ISO) ../isodir
clean:
	rm -f $(OBJS) $(ISO) $(KERN)
