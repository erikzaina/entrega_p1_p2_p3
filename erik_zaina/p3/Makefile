CC      := gcc
CFLAGS  := -Wall -Wextra -O2

all: compilador executor

compilador: compiler.c
	$(CC) $(CFLAGS) -o $@ $<

executor: executor.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f compilador executor

.PHONY: all clean
