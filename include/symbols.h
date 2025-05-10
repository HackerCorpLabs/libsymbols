#ifndef SYMBOLS_H
#define SYMBOLS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "stabs.h"
#include "nlist.h"
#include "mapfile.h"

// Symbol types supported by the library
typedef enum {
    SYMBOL_TYPE_UNKNOWN = 0,
    SYMBOL_TYPE_FUNCTION,
    SYMBOL_TYPE_VARIABLE,
    SYMBOL_TYPE_FILE,
    SYMBOL_TYPE_LINE,
    SYMBOL_TYPE_TYPE
} symbol_type_t;

// Structure for a symbol table entry
typedef struct {
    const char* filename;    // Source file name
    const char* name;        // Symbol name
    int line;               // Line number
    uint16_t address;       // Memory address
    symbol_type_t type;     // Symbol type
    bool owns_strings;      // Whether this entry owns its strings
} symbol_entry_t;

// Structure for the symbol table
typedef struct {
    symbol_entry_t* entries;    // Array of symbol entries
    size_t count;              // Number of entries
    size_t capacity;           // Current capacity
} symbol_table_t;

// Memory segment information
typedef struct {
    uint16_t start_address;  // Starting address of the segment
    uint16_t size;          // Size of the segment in bytes
    uint8_t* data;          // Pointer to the loaded data
    bool is_text;           // Whether this is a text (code) segment
} memory_segment_t;

// Binary loading information
typedef struct {
    memory_segment_t* segments;  // Array of memory segments
    size_t segment_count;        // Number of segments
    uint16_t entry_point;        // Program entry point address
} binary_info_t;

// Create a new symbol table
symbol_table_t* symbols_create(void);

// Free a symbol table and its contents
void symbols_free(symbol_table_t* table);

// Add a new entry to the symbol table
bool symbols_add_entry(symbol_table_t* table, const char* filename, const char* name,
                      int line, uint16_t address, symbol_type_t type);

// Look up a symbol by address
const symbol_entry_t* symbols_lookup_by_address(const symbol_table_t* table, uint16_t address);

// Look up a symbol by name
const symbol_entry_t* symbols_lookup_by_name(const symbol_table_t* table, const char* name);

// Load symbols from an a.out file
bool symbols_load_aout(symbol_table_t* table, const char* filename);

// Load symbols from a STABS .s file
bool symbols_load_stabs(symbol_table_t* table, const char* filename);

// Load symbols from a map file
bool symbols_load_map(symbol_table_t* table, const char* filename);

// Load binary code from a.out file
bool symbols_load_binary(const char* filename, binary_info_t* info);

// Free binary loading information
void symbols_free_binary(binary_info_t* info);

// Get memory segment containing an address
const memory_segment_t* symbols_get_segment(const binary_info_t* info, uint16_t address);

// Get entry point address
uint16_t symbols_get_entry_point(const binary_info_t* info);

// DAP-specific functions

// Find address for a source location
bool symbols_find_address(const symbol_table_t* table, const char* filename,uint16_t *address, int line);

// Get source file for an address
const char* symbols_get_file(const symbol_table_t* table, uint16_t address);

// Get line number for an address
int symbols_get_line(const symbol_table_t* table, uint16_t address);

// Check if an entry represents a line number
bool symbols_is_line_entry(const symbol_entry_t* entry);

// Get next line address for stepping
uint16_t symbols_get_next_line_address(const symbol_table_t* table, uint16_t current_address);

// Comparison function for sorting entries by address
int compare_entries_by_address(const void* a, const void* b);

#endif /* SYMBOLS_H */ 