TARGET_ARCH?=
CC?=
AR?=

ARCH_OBJ=
ARCH_LDFLAGS=
ARCH_INCLUDE_DIRS=
ARCH_LDFILE=
BUILD_OBJ=
include ./kernel/make.config
include ./arch/$(TARGET_ARCH)/make.config

SYSROOT?=$(shell ../get-sysroot.sh)
PREFIX?=/usr/local

INCLUDE_DIRS=$(SYSROOT)$(PREFIX)/include $(ARCH_INCLUDE_DIRS)
ISO=condor.iso
KERN=condor.kern

CFLAGS=-std=gnu99 -ffreestanding -O2 -Wall -Werror -Wextra -g $(addprefix -I,$(INCLUDE_DIRS)) -g
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
	$(CC) -T $(ARCH_LDFILE) -o $@ $(LDFLAGS) $(BUILD_OBJ) $(ARCH_OBJ) $(LIBS) -D _$(TARGET_ARCH)_
iso: $(KERN)
	cp $(KERN) ../isodir/boot/$(KERN)
	i686-grub-mkrescue -o ../$(ISO) ../isodir
clean:
	rm -f $(BUILD_OBJ) $(ARCH_OBJ) ../$(ISO) $(KERN) kern.sym ../condor.sym ./*.i ./*.ii

install-headers:
	mkdir -p $(SYSROOT)$(PREFIX)/include
	cp -rp include $(SYSROOT)$(PREFIX)
install-binaries: $(KERN)

install: install-headers install-binaries
