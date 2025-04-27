#include "../include/mapfile.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <mapfile>\n", argv[0]);
        return 1;
    }

    map_entry_t* entries = NULL;
    size_t count = 0;

    printf("Loading map file: %s\n", argv[1]);
    if (!mapfile_parse_file(argv[1], &entries, &count)) {
        fprintf(stderr, "Failed to parse map file\n");
        return 1;
    }

    printf("Successfully loaded %zu entries:\n", count);
    for (size_t i = 0; i < count; i++) {
        printf("Entry %zu:\n", i);
        printf("  File: %s\n", entries[i].filename);
        printf("  Line: %d\n", entries[i].line);
        printf("  Address: 0x%04X\n", entries[i].address);
        printf("\n");
    }

    mapfile_free_entries(entries, count);
    return 0;
} 