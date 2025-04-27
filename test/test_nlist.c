#include <stdio.h>
#include <stdlib.h>
#include "nlist.h"

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <a.out_file>\n", argv[0]);
        return 1;
    }

    struct aout_nlist* entries = NULL;
    size_t count = 0;

    if (!nlist_parse_file(argv[1], &entries, &count, false)) {
        fprintf(stderr, "Failed to parse nlist entries\n");
        return 1;
    }

    printf("Successfully parsed %zu nlist entries:\n\n", count);
    
    for (size_t i = 0; i < count; i++) {
        const struct aout_nlist* entry = &entries[i];
        printf("Entry %zu:\n", i);
        printf("  Name: %s\n", entry->name);
        printf("  Type: 0x%02x\n", entry->type);
        printf("  Value: 0x%04x\n", entry->value);
        printf("  Other: 0x%02x\n", entry->other);
        printf("  Desc: 0x%04x\n", entry->desc);
        printf("\n");
    }

    nlist_free_entries(entries, count);
    return 0;
} 