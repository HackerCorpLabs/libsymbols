#include "symbols.h"
#include "stabs.h"
#include "nlist.h"
#include "mapfile.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include <a.out.h>  // For struct exec definition

// Symbol table structure
struct symbol_table {
    symbol_entry_t* entries;
    size_t count;
    size_t capacity;
};

// Comparison function for qsort and bsearch
int compare_entries_by_address(const void* a, const void* b) {
    const symbol_entry_t* entry_a = (const symbol_entry_t*)a;
    const symbol_entry_t* entry_b = (const symbol_entry_t*)b;
    return entry_a->address - entry_b->address;
}

// Helper function to map STABS type to symbol type
static symbol_type_t map_stabs_type(int type_code) {
    switch (type_code) {
        case N_FUN: return SYMBOL_TYPE_FUNCTION;
        case N_GSYM:
        case N_LSYM: return SYMBOL_TYPE_VARIABLE;
        case N_SO: return SYMBOL_TYPE_FILE;
        case N_SLINE: return SYMBOL_TYPE_LINE;
        case N_TYPE: return SYMBOL_TYPE_TYPE;
        default: return SYMBOL_TYPE_UNKNOWN;
    }
}

// Helper function to map nlist type to symbol type
static symbol_type_t map_nlist_type(int type) {
    switch (type & 0x1e) {  // Mask out N_EXT bit
        case N_TEXT: return SYMBOL_TYPE_FUNCTION;
        case N_DATA:
        case N_BSS: return SYMBOL_TYPE_VARIABLE;
        default: return SYMBOL_TYPE_UNKNOWN;
    }
}

// Create a new symbol table
symbol_table_t* symbols_create(void) {
    symbol_table_t* table = malloc(sizeof(symbol_table_t));
    if (!table) return NULL;

    table->capacity = 16;
    table->count = 0;
    table->entries = malloc(table->capacity * sizeof(symbol_entry_t));
    if (!table->entries) {
        free(table);
        return NULL;
    }

    return table;
}

// Free a symbol table and its contents
void symbols_free(symbol_table_t* table) {
    if (!table) return;

    // Free all entries
    for (size_t i = 0; i < table->count; i++) {
        symbol_entry_t* entry = &table->entries[i];
        if (entry->owns_strings) {
            if (entry->name) free((void*)entry->name);
            if (entry->filename) free((void*)entry->filename);
        }
    }

    // Free the entries array
    free(table->entries);
    free(table);
}

// Add a new entry to the symbol table
bool symbols_add_entry(symbol_table_t* table, const char* filename, const char* name,
                      int line, uint16_t address, symbol_type_t type) {
    if (!table) return false;

    // Resize if needed
    if (table->count >= table->capacity) {
        size_t new_capacity = table->capacity * 2;
        symbol_entry_t* new_entries = realloc(table->entries, 
                                            new_capacity * sizeof(symbol_entry_t));
        if (!new_entries) return false;
        table->entries = new_entries;
        table->capacity = new_capacity;
    }

    // Initialize new entry
    symbol_entry_t* entry = &table->entries[table->count];
    
    // Set string ownership to true since we're duplicating the strings
    entry->owns_strings = true;
    
    if (filename) {
        entry->filename = strdup(filename);
        if (!entry->filename) return false;
    } else {
        entry->filename = NULL;
    }

    if (name) {
        entry->name = strdup(name);
        if (!entry->name) {
            free((void*)entry->filename);
            return false;
        }
    } else {
        entry->name = NULL;
    }
    
    entry->line = line;
    entry->address = address;
    entry->type = type;

    table->count++;
    return true;
}

// Load symbols from a STABS .s file
bool symbols_load_stabs(symbol_table_t* table, const char* filename) {
    if (!table || !filename) return false;

    stab_entry_t* entries = NULL;
    size_t count = 0;

    if (!stabs_parse_file(filename, &entries, &count)) {
        return false;
    }

    bool success = true;
    for (size_t i = 0; i < count; i++) {
        symbol_entry_t entry = {
            .filename = entries[i].filename,
            .name = entries[i].name,
            .line = entries[i].line,
            .address = entries[i].value,
            .type = map_stabs_type(entries[i].type_code),
            .owns_strings = false
        };

        if (!symbols_add_entry(table, entry.filename, entry.name, 
                             entry.line, entry.address, entry.type)) {
            success = false;
            break;
        }
    }

    stabs_free_entries(entries, count);
    return success;
}

// Load symbols from an a.out file
bool symbols_load_aout(symbol_table_t* table, const char* filename) {
    if (!table || !filename) return false;

    // Parse nlist entries
    struct aout_nlist* entries = NULL;
    size_t count = 0;
    if (!nlist_parse_file(filename, &entries, &count, false)) {
        return false;
    }

    
    // Ad a N_SO symbol to tell that source files begins here
    // If not, symbols_find_address() will not find this file
    symbol_entry_t filename_begin = {
            .filename = filename,
            .name = NULL,
            .line = 0,
            .address = 0,
            .type = SYMBOL_TYPE_FILE,
            .owns_strings = false
    };

    symbols_add_entry(table, filename_begin.filename, filename_begin.name, filename_begin.line, filename_begin.address, filename_begin.type);

    bool success = true;
    for (size_t i = 0; i < count; i++) {        
        // Skip undefined symbols
        if ((entries[i].n_type & 0x1e) == N_UNDF) {
            continue;
        }

        symbol_entry_t entry = {
            .filename = NULL,
            .name = entries[i].n_un.n_name,
            .line = 0,
            .address = entries[i].n_value,
            .type = map_nlist_type(entries[i].n_type),
            .owns_strings = false
        };

        if (!symbols_add_entry(table, entry.filename, entry.name, 
                             entry.line, entry.address, entry.type)) {
            success = false;
            break;
        }
    }

    nlist_free_entries(entries, count);

    // Add a N_SO symbol to tell that source files begins here
    // If not, symbols_find_address() will not find this file
    symbol_entry_t filename_done = {
            .filename = "",
            .name = NULL,
            .line = 0,
            .address = 0,
            .type = SYMBOL_TYPE_FILE,
            .owns_strings = false
    };

    // Add an "empty" N_SO to tell that source files ends here
    symbols_add_entry(table, filename_done.filename, filename_done.name, filename_done.line, filename_done.address, filename_done.type);

    return success;
}

// Load symbols from a map file
bool symbols_load_map(symbol_table_t* table, const char* filename) {
    if (!table || !filename) return false;

    map_entry_t* entries = NULL;
    size_t count = 0;

    if (!mapfile_parse_file(filename, &entries, &count)) {
        return false;
    }

    // Ad a N_SO symbol to tell that source files begins here
    // If not, symbols_find_address() will not find this file
    symbol_entry_t filename_begin = {
            .filename = entries[0].filename,
            .name = NULL,
            .line = 0,
            .address = 0,
            .type = SYMBOL_TYPE_FILE,
            .owns_strings = false
    };

    symbols_add_entry(table, filename_begin.filename, filename_begin.name, filename_begin.line, filename_begin.address, filename_begin.type);

    bool success = true;
    for (size_t i = 0; i < count; i++) {
        symbol_entry_t entry = {
            .filename = entries[i].filename,
            .name = NULL,
            .line = entries[i].line,
            .address = entries[i].address,
            .type = SYMBOL_TYPE_LINE,
            .owns_strings = false
        };

        if (!symbols_add_entry(table, entry.filename, entry.name, 
                             entry.line, entry.address, entry.type)) {
            success = false;
            break;
        }
    }

    mapfile_free_entries(entries, count);

    // Add a N_SO symbol to tell that source files begins here
    // If not, symbols_find_address() will not find this file
    symbol_entry_t filename_done = {
            .filename = "",
            .name = NULL,
            .line = 0,
            .address = 0,
            .type = SYMBOL_TYPE_FILE,
            .owns_strings = false
    };

    // Add an "empty" N_SO to tell that source files ends here
    symbols_add_entry(table, filename_done.filename, filename_done.name, filename_done.line, filename_done.address, filename_done.type);

    return success;
}

// Look up a symbol by address
const symbol_entry_t* symbols_lookup_by_address(const symbol_table_t* table, uint16_t address) {
    if (!table || table->count == 0) return NULL;

    // Create a temporary entry for searching
    symbol_entry_t key = { .address = address };
    
    // Use binary search to find the entry
    symbol_entry_t* result = bsearch(&key, table->entries, table->count,
                                   sizeof(symbol_entry_t), compare_entries_by_address);
    
    return result;
}

// Dump all symbols to stdout for debugging
void symbols_dump_all(const symbol_table_t* table) {
    if (!table) return;

    printf("Symbol Table (%zu entries):\n", table->count);
    printf("----------------------------\n");

    for (size_t i = 0; i < table->count; i++) {
        const symbol_entry_t* entry = &table->entries[i];
        printf("Entry %zu:\n", i);
        printf("  Name: %s\n", entry->name ? entry->name : "(none)");
        printf("  Type: %d\n", entry->type);
        printf("  File: %s\n", entry->filename ? entry->filename : "(none)");
        printf("  Line: %d\n", entry->line);
        printf("  Address: 0x%04X\n", entry->address);
        printf("\n");
    }
}

// Find address for a source location
bool symbols_find_address(const symbol_table_t* table, const char* filename,uint16_t *address, int line) {
    if (!table || !filename) return 0;

    // First, find the file entry
    const symbol_entry_t* file_entry = NULL;
    for (size_t i = 0; i < table->count; i++) {
        if (table->entries[i].type == SYMBOL_TYPE_FILE &&
            table->entries[i].filename != NULL &&
            strcmp(table->entries[i].filename, filename) == 0) {
            file_entry = &table->entries[i];
            break;
        }
    }
    if (!file_entry) return false;

    // Then find the closest line number entry
    uint16_t closest_address = 0;
    int closest_line_diff = INT_MAX;

    for (size_t i = 0; i < table->count; i++) {
        if (table->entries[i].type == SYMBOL_TYPE_LINE &&
            strcmp(table->entries[i].filename, filename) == 0) {
            int line_diff = abs(table->entries[i].line - line);
            if (line_diff < closest_line_diff) {
                closest_line_diff = line_diff;
                closest_address = table->entries[i].address;
            }
        }
    }

    *address = closest_address;;
    return true;
}

// Get source file for an address
const char* symbols_get_file(const symbol_table_t* table, uint16_t address) {
    if (!table) return NULL;

    // Find the closest line number entry
    const symbol_entry_t* closest = NULL;
    uint16_t closest_diff = UINT16_MAX;

    for (size_t i = 0; i < table->count; i++) {
        if (table->entries[i].type == SYMBOL_TYPE_LINE) {
            uint16_t diff = abs(table->entries[i].address - address);
            if (diff < closest_diff) {
                closest_diff = diff;
                closest = &table->entries[i];
            }
        }
    }

    return closest ? closest->filename : NULL;
}

// Get line number for an address
int symbols_get_line(const symbol_table_t* table, uint16_t address) {
    if (!table) return 0;

    // Find the closest line number entry
    const symbol_entry_t* closest = NULL;
    uint16_t closest_diff = UINT16_MAX;

    for (size_t i = 0; i < table->count; i++) {
        if (table->entries[i].type == SYMBOL_TYPE_LINE) {
            uint16_t diff = abs(table->entries[i].address - address);
            if (diff < closest_diff) {
                closest_diff = diff;
                closest = &table->entries[i];
            }
        }
    }

    return closest ? closest->line : 0;
}

// Check if an entry represents a line number
bool symbols_is_line_entry(const symbol_entry_t* entry) {
    return entry && entry->type == SYMBOL_TYPE_LINE;
}

/// @brief Finds the memory address of the next source code line for stepping through code
/// @param table Pointer to the symbol table containing line number to address mappings
/// @param current_address The current memory address to find the next line from
/// @return The memory address of the next line in the same source file, or 0 if no next line exists
uint16_t symbols_get_next_line_address(const symbol_table_t* table, uint16_t current_address) {
    if (!table) return 0;

    // Find the current line entry
    const symbol_entry_t* current = NULL;
    uint16_t closest_diff = UINT16_MAX;

    for (size_t i = 0; i < table->count; i++) {
        if (table->entries[i].type == SYMBOL_TYPE_LINE) {
            uint16_t diff = abs(table->entries[i].address - current_address);
            if (diff < closest_diff) {
                closest_diff = diff;
                current = &table->entries[i];
            }
        }
    }

    if (!current) return 0;

    // Find the next line entry in the same file
    const symbol_entry_t* next = NULL;
    uint16_t next_address = UINT16_MAX;

    for (size_t i = 0; i < table->count; i++) {
        if (table->entries[i].type == SYMBOL_TYPE_LINE &&
            strcmp(table->entries[i].filename, current->filename) == 0 &&
            table->entries[i].line > current->line &&
            table->entries[i].address < next_address) {
            next = &table->entries[i];
            next_address = table->entries[i].address;
        }
    }

    return next ? next->address : 0;
}

// Load binary code from a.out file
bool symbols_load_binary(const char* filename, binary_info_t* info) {
    if (!filename || !info) return false;

    FILE* file = fopen(filename, "rb");
    if (!file) return false;

    // Read a.out header
    struct exec header;
    if (fread(&header, sizeof(header), 1, file) != 1) {
        fclose(file);
        return false;
    }

    // Initialize binary info
    info->segment_count = 0;
    info->segments = NULL;
    info->entry_point = header.a_entry;

    // Allocate segments array
    info->segments = malloc(2 * sizeof(memory_segment_t)); // Text and data segments
    if (!info->segments) {
        fclose(file);
        return false;
    }

    // Load text segment
    info->segments[0].start_address = header.a_text;
    info->segments[0].size = header.a_text;
    info->segments[0].is_text = true;
    info->segments[0].data = malloc(header.a_text);
    if (!info->segments[0].data) {
        free(info->segments);
        fclose(file);
        return false;
    }

    if (fread(info->segments[0].data, header.a_text, 1, file) != 1) {
        free(info->segments[0].data);
        free(info->segments);
        fclose(file);
        return false;
    }

    // Load data segment
    info->segments[1].start_address = header.a_data;
    info->segments[1].size = header.a_data;
    info->segments[1].is_text = false;
    info->segments[1].data = malloc(header.a_data);
    if (!info->segments[1].data) {
        free(info->segments[0].data);
        free(info->segments);
        fclose(file);
        return false;
    }

    if (fread(info->segments[1].data, header.a_data, 1, file) != 1) {
        free(info->segments[1].data);
        free(info->segments[0].data);
        free(info->segments);
        fclose(file);
        return false;
    }

    info->segment_count = 2;
    fclose(file);
    return true;
}

// Free binary loading information
void symbols_free_binary(binary_info_t* info) {
    if (!info) return;

    for (size_t i = 0; i < info->segment_count; i++) {
        free(info->segments[i].data);
    }
    free(info->segments);
    info->segments = NULL;
    info->segment_count = 0;
    info->entry_point = 0;
}

// Get memory segment containing an address
const memory_segment_t* symbols_get_segment(const binary_info_t* info, uint16_t address) {
    if (!info) return NULL;

    for (size_t i = 0; i < info->segment_count; i++) {
        if (address >= info->segments[i].start_address &&
            address < info->segments[i].start_address + info->segments[i].size) {
            return &info->segments[i];
        }
    }
    return NULL;
}

// Get entry point address
uint16_t symbols_get_entry_point(const binary_info_t* info) {
    return info ? info->entry_point : 0;
} 