.POSIX:
CC       = cc
CFLAGS   = -std=c17 -Werror=vla -Wextra -Wall -static -Wno-unused-parameter
CPPFLAGS =
LDFLAGS  = -ggdb3
LDLIBS   =
PREFIX   = /usr/local

all: executable

debug: CFLAGS += -DDEBUG -g
debug: executable

release: CFLAGS += -s
release: executable

executable: linux-socket-bfosb

linux-socket-bfosb:
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -o build/$@ src/linux-socket-bfosb.c $(LDLIBS)

clean:
	rm -rf *.o