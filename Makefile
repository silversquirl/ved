.POSIX:
.PHONY: all clean

CFLAGS := -Wall -Werror
LDFLAGS :=

all: ved

clean:
	rm -f ved *.o

ved: ved.o editor.o rope.o
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c %.h
	$(CC) -c $< $(CFLAGS)
