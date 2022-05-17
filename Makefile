CFLAGS := -Wall -g
PREFIX := /usr

.PHONY: all separate embedded install-separate install-embedded clean

all: separate embedded

# Set path to separate LD_PRELOAD library path at compile-time
separate: PRELOAD_SO_PATH = $(PREFIX)/libexec/nerfed/libeat-function.so
separate: CFLAGS += -DPRELOAD_SO_PATH=$(PRELOAD_SO_PATH)
separate: nerfed-separate libeat-function.so

embedded: nerfed-embedded

nerfed-separate: nerfed-separate.o

nerfed-separate.o:
	$(COMPILE.c) $(OUTPUT_OPTION) nerfed.c

nerfed-embedded.o:
	$(COMPILE.c) $(OUTPUT_OPTION) nerfed.c

nerfed-embedded: nerfed-embedded.o libeat-function-embed.o

libeat-function.so: libeat-function.c
	$(CC) $(CFLAGS) -fPIC -shared -ldl -o $@ $<

libeat-function-embed.c: libeat-function.so
	( \
		set -e; \
		echo "#include <stdint.h>"; \
		echo "#include <sys/types.h>"; \
		echo ""; \
		echo "const uint8_t libeat_function_so[] = {"; \
		od -An -t 'x1' -v $< | sed 's/^ /\t0x/; s/ /, 0x/g ; s/$$/,/'; \
		echo "};"; \
		echo ""; \
		echo "const size_t libeat_function_so_len = sizeof(libeat_function_so);"; \
	) > $@

install-separate: separate
	install -m 0755 -d $(PREFIX)/bin
	install -m 0755 nerfed-separate $(PREFIX)/bin/nerfed
	install -m 0755 -d $(PREFIX)/libexec/nerfed
	install -m 0755 -t $(PREFIX)/libexec/nerfed libeat-function.so

install-embedded: embedded
	install -m 0755 -d $(PREFIX)/bin
	install -m 0755 nerfed-embedded $(PREFIX)/bin/nerfed

clean:
	rm -f *.o *.so libeat-function-embed.c nerfed-separate nerfed-embedded
