#include <stdio.h>
#include <stdlib.h>
#include "aout.h"

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <a.out_file>\n", argv[0]);
        return 1;
    }

    aout_entry_t* entries = NULL;
    size_t count = 0;

    if (!aout_parse_file(argv[1], &entries, &count)) {
        fprintf(stderr, "Failed to parse a.out entries\n");
        return 1;
    }

    printf("Successfully parsed %zu a.out entries:\n\n", count);
    
    for (size_t i = 0; i < count; i++) {
        const aout_entry_t* entry = &entries[i];
        printf("Entry %zu:\n", i);
        printf("  Name: %s\n", entry->name);
        printf("  Type: 0x%02x\n", entry->type);
        printf("  Value: 0x%04x\n", entry->value);
        printf("  Desc: 0x%02x\n", entry->desc);
        printf("\n");
    }

    aout_free_entries(entries, count);
    return 0;
} 