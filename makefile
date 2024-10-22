CC=gcc
CFLAGS=-Wall -Wextra -Wno-implicit-fallthrough -std=gnu17 -fPIC -O2
LDFLAGS=-shared -Wl,--wrap=malloc -Wl,--wrap=calloc -Wl,--wrap=realloc -Wl,--wrap=reallocarray -Wl,--wrap=free -Wl,--wrap=strdup -Wl,--wrap=strndup

sources=nand.c memory_tests.c
objects=$(sources:.c=.o)
lib=libnand.so
headers=nand.h memory_tests.h

.PHONY: all clean

all: $(lib)

$(lib): $(objects)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c $(headers)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(objects) $(lib)