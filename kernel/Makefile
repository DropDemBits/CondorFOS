TARGET_ARCH=i386

ARCH_OBJ=
ARCH_LDFLAGS=
ARCH_INCLUDE_DIRS=
ARCH_LDFILE=
BUILD_OBJ=
include ./kernel/make.config
include ./arch/$(TARGET_ARCH)/make.config

SYSROOT?=$(shell ../get-sysroot.sh)
PREFIX?=/usr/local

INCLUDE_DIRS=../grub/include $(SYSROOT)$(PREFIX)/include $(ARCH_INCLUDE_DIRS)
ISO=condor.iso
KERN=condor.kern

CC=i686-elf-gcc
CFLAGS=-std=gnu99 -ffreestanding -O2 -Wall -Werror -Wextra -g $(addprefix -I,$(INCLUDE_DIRS))
LDFLAGS=-ffreestanding -O2 -L$(SYSROOT)$(PREFIX)/lib $(addprefix -I,$(INCLUDE_DIRS)) $(ARCH_LDFLAGS)
LIBS=-nostdlib -lgcc -lk

CRTI_OBJ:=arch/$(TARGET_ARCH)/crti.o
CRTBEGIN_OBJ:=$(shell $(CC) -print-file-name=crtbegin.o)
CRTOBJ_OBJ:=$(shell $(CC) -print-file-name=crtend.o)
CRTN_OBJ:=arch/$(TARGET_ARCH)/crtn.o

OBJ_LINKS:= \
$(CRTI_OBJ) \
$(CRTBEGIN_OBJ) \
$(ARCH_OBJ) \
$(BUILD_OBJ) \
$(CRTEND_OBJ) \
$(CRTN_OBJ) \

.PHONY: iso clean $(KERN)

%.o: %.S
	$(CC) -c -o $@ $< $(addprefix -I,$(INCLUDE_DIRS))
%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS) $(addprefix -I,$(INCLUDE_DIRS))

$(KERN): $(OBJ_LINKS)
	$(CC) -T $(ARCH_LDFILE) -o $@ $(LDFLAGS) $(BUILD_OBJ) $(ARCH_OBJ) $(LIBS)
iso: $(KERN)
	cp $(KERN) ../isodir/boot/$(KERN)
	i686-grub-mkrescue -o ../$(ISO) ../isodir
clean:
	rm -f $(OBJS) $(ARCH_OBJ) ../$(ISO) $(KERN)

install-headers:
	mkdir -p $(SYSROOT)$(PREFIX)/include
	cp -rp include $(SYSROOT)$(PREFIX)
install-binaries: $(KERN)

install: install-headers install-binaries
