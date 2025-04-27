#include <stdio.h>
#include <stdlib.h>
#include "symbols.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <symbol_file>\n", argv[0]);
        return 1;
    }

    symbols_table_t* table = symbols_create();
    if (!table) {
        fprintf(stderr, "Failed to create symbol table\n");
        return 1;
    }

    // Try to load the file based on extension
    const char* filename = argv[1];
    bool success = false;
    
    if (strstr(filename, ".s")) {
        success = symbols_load_stabs(table, filename);
    } else if (strstr(filename, ".out")) {
        success = symbols_load_nlist(table, filename);
    } else if (strstr(filename, ".map")) {
        success = symbols_load_map(table, filename);
    } else {
        fprintf(stderr, "Unknown file type: %s\n", filename);
        symbols_free(table);
        return 1;
    }

    if (!success) {
        fprintf(stderr, "Failed to load symbols from %s\n", filename);
        symbols_free(table);
        return 1;
    }

    // Dump all symbols
    printf("Symbols loaded successfully. Dumping contents:\n");
    symbols_dump_all(table);

    symbols_free(table);
    return 0;
} 