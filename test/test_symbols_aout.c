#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/symbols.h"

static void print_usage(const char* program) {
    fprintf(stderr, "Usage: %s <a.out file> [--dump-code]\n", program);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  --dump-code    Dump binary code in octal format\n");
}

static void dump_code(const binary_info_t* info) {
    if (!info || !info->segments) {
        fprintf(stderr, "No binary code loaded\n");
        return;
    }

    printf("Binary Code Dump (octal):\n");
    printf("========================\n");

    for (size_t i = 0; i < info->segment_count; i++) {
        const memory_segment_t* seg = &info->segments[i];
        printf("\nSegment %zu: %s (start: %06o, size: %06o)\n",
               i, seg->is_text ? "TEXT" : "DATA",
               seg->start_address, seg->size);

        // Print 16 bytes per line
        for (uint16_t addr = 0; addr < seg->size; addr += 16) {
            // Print address
            printf("%06o:", seg->start_address + addr);

            // Print values
            for (int j = 0; j < 16 && (addr + j) < seg->size; j++) {
                printf(" %03o", seg->data[addr + j]);
            }
            printf("\n");
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    const char* filename = NULL;
    bool should_dump_code = false;

    // Parse options
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--dump-code") == 0) {
            should_dump_code = true;
        } else if (!filename) {
            filename = argv[i];
        } else {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        }
    }

    if (!filename) {
        fprintf(stderr, "No input file specified\n");
        print_usage(argv[0]);
        return 1;
    }

    // Create symbol table
    symbol_table_t* table = symbols_create();
    if (!table) {
        fprintf(stderr, "Failed to create symbol table\n");
        return 1;
    }

    // Load symbols
    if (!symbols_load_aout(table, filename)) {
        fprintf(stderr, "Failed to load symbols from %s\n", filename);
        symbols_free(table);
        return 1;
    }

    // Load binary if requested
    binary_info_t binary_info;
    if (should_dump_code) {
        if (!symbols_load_binary(filename, &binary_info)) {
            fprintf(stderr, "Failed to load binary from %s\n", filename);
            symbols_free(table);
            return 1;
        }
    }

    // Dump symbols
    printf("Symbols from %s:\n", filename);
    printf("================\n\n");

    for (size_t i = 0; i < table->count; i++) {
        const symbol_entry_t* entry = &table->entries[i];
        printf("Symbol %zu:\n", i);
        printf("  Name: %s\n", entry->name ? entry->name : "(none)");
        printf("  Type: %d\n", entry->type);
        printf("  File: %s\n", entry->filename ? entry->filename : "(none)");
        printf("  Line: %d\n", entry->line);
        printf("  Address: %06o\n", entry->address);
        printf("\n");
    }

    // Dump code if requested
    if (should_dump_code) {
        dump_code(&binary_info);
        symbols_free_binary(&binary_info);
    }

    symbols_free(table);
    return 0;
} 