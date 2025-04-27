CC = gcc
CFLAGS = -Wall -Wextra -I../include
LDFLAGS = -L.. -lsymbols

all: test_mapfile test_performance test_symbols_aout

test_mapfile: test_mapfile.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

test_performance: test_performance.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

test_symbols_aout: test_symbols_aout.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f test_mapfile test_performance test_symbols_aout

.PHONY: all clean 