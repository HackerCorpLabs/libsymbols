CC = gcc
CFLAGS = -Wall -Wextra -I../include
LDFLAGS = -L.. -lsymbols

# Define the library and header file dependencies
LIB_SYMBOLS = ../libsymbols.a  # Assuming a static library, adjust if it's .so
HDR_FILES = $(wildcard ../include/*.h)

all: test_mapfile test_performance test_symbols_aout dump_header

test_mapfile: test_mapfile.c $(LIB_SYMBOLS) $(HDR_FILES)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

test_performance: test_performance.c $(LIB_SYMBOLS) $(HDR_FILES)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

test_symbols_aout: test_symbols_aout.c $(LIB_SYMBOLS) $(HDR_FILES)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

dump_header: dump_header.c $(LIB_SYMBOLS) $(HDR_FILES)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f test_mapfile test_performance test_symbols_aout dump_header

.PHONY: all clean 