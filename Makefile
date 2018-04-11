.POSIX:
.PHONY: all clean

CFLAGS := -Wall -Werror $(shell pkg-config --cflags pangoxft x11) $(DEBUG)
LDFLAGS := $(shell pkg-config --libs pangoxft x11)

all: ved

clean:
	rm -f ved *.o

ved: ved.o ui.o editor.o rope.o
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c %.h
	$(CC) -c $< $(CFLAGS)
