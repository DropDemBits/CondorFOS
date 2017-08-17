CC ?=
CFLAGS :=
OBJS = \
	test.o \

%.o: %.S
	$(CC) -c $< -o bin/$@ $(CFLAGS)	

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

.PHONY: install install-headers clean

test.bin: $(OBJS)
	objcopy --only-section=.text --output-target binary ./bin/test.o ./bin/$@

install: test.bin
	
install-headers:
	

clean:
	rm -rf bin
	mkdir bin
