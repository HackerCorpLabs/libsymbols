#include <stdio.h>
#include <stdlib.h>
#include "stabs.h"

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <stabs_file>\n", argv[0]);
        return 1;
    }

    stab_entry_t* entries = NULL;
    size_t count = 0;

    if (!stabs_parse_file(argv[1], &entries, &count)) {
        fprintf(stderr, "Failed to parse STABS file: %s\n", argv[1]);
        return 1;
    }

    printf("Successfully parsed %zu STABS entries:\n\n", count);
    
    for (size_t i = 0; i < count; i++) {
        const stab_entry_t* entry = &entries[i];
        printf("Entry %zu:\n", i);
        printf("  Name: %s\n", entry->name);
        printf("  Descriptor: %c\n", entry->desc);
        printf("  Type: %s\n", entry->type);
        printf("  Type Code: 0x%02x\n", entry->type_code);
        printf("  Value: 0x%04x\n", entry->value);
        if (entry->filename) {
            printf("  File: %s\n", entry->filename);
        }
        printf("\n");
    }

    stabs_free_entries(entries, count);
    return 0;
} 