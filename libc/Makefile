TARGET_ARCH?=
CC?=
AR?=
DESTDIR?=
PREFIX?=/usr/local

OBJ=
CFLAGS=-Wall -Wextra -ffreestanding
CPPFLAGS=-D__LIBC_BUILD -I$(DESTDIR)$(PREFIX)/include
LIBK_CFLAGS=$(CFLAGS) -ffreestanding -fbuiltin
LIBK_CPPFLAGS=$(CPPFLAGS) -D__LIBK_BUILD -Iinclude

include ./arch/$(TARGET_ARCH)/make.config

OBJ:= $(OBJ) \
stdio/printf.o \
stdio/putchar.o \
stdio/puts.o \
stdlib/abort.o \
stdlib/exit.o \
string/memcmp.o \
string/memcpy.o \
string/memmove.o \
string/memset.o \
string/strlen.o \

LIBK_OBJ:=$(OBJ:.o=.libk.o)

BINARIES=libk.a #libc.a

%.o: %.c
	$(CC) -c -o $@ $< -std=gnu11 $(CFLAGS) $(CPPFLAGS)
%.o: %.S
	$(CC) -c -o $@ $< $(CFLAGS) $(CPPFLAGS)

%.libk.o: %.c
	$(CC) -c -o $@ $< -std=gnu11 $(LIBK_CFLAGS) $(LIBK_CPPFLAGS)
%.libk.o: %.S
	$(CC) -c -o $@ $< $(LIBK_CFLAGS) $(LIBK_CPPFLAGS)

libc.a: $(OBJ)
	$(AR) rcs $@ $(OBJ)
libk.a: $(LIBK_OBJ)
	$(AR) rcs $@ $(LIBK_OBJ)
clean:
	rm -f $(OBJ) $(LIBK_OBJ) $(BINARIES) *.o */*.o */*/*.o

install-binaries: $(BINARIES)
	mkdir -p $(DESTDIR)$(PREFIX)/lib/
	cp $(BINARIES) $(DESTDIR)$(PREFIX)/lib/
install-headers:
	mkdir -p $(DESTDIR)$(PREFIX)/include
	cp -rp ./include $(DESTDIR)$(PREFIX)

install: install-headers install-binaries
