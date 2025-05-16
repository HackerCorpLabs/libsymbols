#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
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

    printf("\n");
    printf("Binary Code Dump (octal):\n");
    printf("=========================\n");

    for (size_t i = 0; i < info->segment_count; i++) {
        const memory_segment_t* seg = &info->segments[i];
        printf("\nSegment %zu: %s (start: %06o, size: %06o)\n",
               i, seg->is_text ? "TEXT" : "DATA",
               seg->start_address, seg->size);

        // Print header with actual addresses (only as many as needed for the first line)
        size_t segment_bytes = seg->size * 2;
        // Always show all 16 columns for consistent formatting
        int header_cols = 16;
        printf("Offset(o)  ");
        for (int col = 0; col < header_cols; col++) printf(" %03o", col);
        printf("  | Decoded text\n");
        
        // Print a line of dashes matching the exact width of the header
        printf("%s", "---------  ");
        for (int col = 0; col < header_cols; col++) printf(" ---");
        printf("  | ----------------\n");

        // Print 16 bytes per line
        for (size_t addr = 0; addr < segment_bytes; addr += 16) {
            // Print address
            printf("%06o     ", seg->start_address + addr);

            // Print octal values
            for (int j = 0; j < 16; j++) {
                if ((addr + j) < segment_bytes)
                    printf(" %03o", seg->data[addr + j]);
                else
                    printf("    ");
            }
            
            // Ensure consistent spacing before ASCII section
            printf("  | ");
            
            // Print ASCII representation with byte swap in pairs
            for (int j = 0; j < 16; j += 2) {
                if ((addr + j + 1) < segment_bytes) {
                    uint8_t b = seg->data[addr + j + 1];
                    uint8_t a = seg->data[addr + j];
                    printf("%c%c", isprint(b) ? b : '.', isprint(a) ? a : '.');
                } else if ((addr + j) < segment_bytes) {
                    uint8_t a = seg->data[addr + j];
                    printf("%c ", isprint(a) ? a : '.');
                } else {
                    printf("  ");
                }
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

#if 0 // This is just wrong because its using a "common" enum for type. Refactor to use STABS types    
    // Dump symbols
    printf("Symbols from %s:\n", filename);
    printf("================\n\n");

    for (size_t i = 0; i < table->count; i++) {
        const symbol_entry_t* entry = &table->entries[i];
        printf("Symbol %zu:\n", i);
        printf("  Name: %s\n", entry->name ? entry->name : "(none)");
        printf("  Type: 0x%02x (%s)\n", entry->type, get_symbol_type(entry->type));
        printf("  Desc: 0x%02x (%s)\n", entry->desc, entry->desc ? get_symbol_desc(entry->type) : "Not used");
        printf("  File: %s\n", entry->filename ? entry->filename : "(none)");
        printf("  Line: %d\n", entry->line);
        printf("  Address: %06o\n", entry->address);
        printf("\n");
    }
#endif

    // Dump code if requested
    if (should_dump_code) {
        dump_code(&binary_info);
        symbols_free_binary(&binary_info);
    }

    symbols_free(table);
    return 0;
} 