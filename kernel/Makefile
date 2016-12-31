TARGET_ARCH=i386

ARCH_OBJ=
ARCH_LDFLAGS=
ARCH_INCLUDE_DIRS=
ARCH_LDFILE=
BUILD_OBJ=
include ./kernel/make.config
include ./arch/$(TARGET_ARCH)/make.config

INCLUDE_DIRS=../grub/include ./include ./include/kernel $(ARCH_INCLUDE_DIRS)
ISO=condor.iso
KERN=condor.kern

CC=i686-elf-gcc
CFLAGS=-std=gnu99 -ffreestanding -O2 -Wall -Wextra -Werror -g
LDFLAGS=-ffreestanding -O2 -nostdlib -lgcc $(addprefix -I,$(INCLUDE_DIRS)) $(ARCH_LDFLAGS)

.PHONY: iso clean $(KERN)

%.o: %.S
	$(CC) -c -o $@ $< $(addprefix -I,$(INCLUDE_DIRS))
%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS) $(addprefix -I,$(INCLUDE_DIRS))

$(KERN): $(BUILD_OBJ) $(ARCH_OBJ)
	$(CC) -T $(ARCH_LDFILE) -o $@ $(LDFLAGS) $(BUILD_OBJ) $(ARCH_OBJ)
iso: $(KERN)
	cp $(KERN) ../isodir/boot/$(KERN)
	i686-grub-mkrescue -o ../$(ISO) ../isodir
clean:
	rm -f $(OBJS) $(ARCH_OBJ) ../$(ISO) $(KERN)
