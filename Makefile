.POSIX:
.PHONY: all clean

CFLAGS := -D_POSIX_C_SOURCE=200809L -Wall -Werror $(shell pkg-config --cflags pangocairo x11 xi) $(DEBUG)
LDFLAGS := $(shell pkg-config --libs pangocairo x11 xi)

SOURCE := $(wildcard *.c)
OBJECT := $(patsubst %.c,%.o,$(SOURCE))

all: ved

clean:
	rm -f ved *.o

ved: $(OBJECT)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c %.h
	$(CC) -c $< $(CFLAGS)
