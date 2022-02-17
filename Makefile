.POSIX:
CC       = cc
CFLAGS   = -std=c17 -Werror=vla -Wextra -Wall -O0 -Wno-unused-parameter
CPPFLAGS =
LDFLAGS  = -static
LDLIBS   =
PREFIX   = /usr/local

all: executable

debug: CFLAGS += -DDEBUG -g
debug: executable

release: CFLAGS += -s
release: executable

executable: linux-socket-bfosb-x86 linux-socket-bfosb-x64

linux-socket-bfosb-x86: build
	$(CC) $(CFLAGS) -m32 $(CPPFLAGS) $(LDFLAGS) -o build/$@ src/linux-socket-bfosb.c $(LDLIBS)

linux-socket-bfosb-x64: build
	$(CC) $(CFLAGS) -m64 $(CPPFLAGS) $(LDFLAGS) -o build/$@ src/linux-socket-bfosb.c $(LDLIBS)

build:
	mkdir -p $@

clean:
	rm -rf *.o