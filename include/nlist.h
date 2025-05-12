#ifndef NLIST_H
#define NLIST_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// a.out format definitions
#define A_MAGIC1 0407    // Normal executable
#define A_MAGIC2 0410    // Read-only text
#define A_MAGIC3 0411    // Separated I&D
#define A_MAGIC4 0405    // Read-only shareable
#define A_MAGIC5 0430    // Auto-overlay (nonseparate)
#define A_MAGIC6 0431    // Auto-overlay (separate)

// Symbol type definitions

/**
 * @brief Undefined symbol (0x0)
 * 
 * Symbol is declared but not defined in this file (e.g., external functions or variables).
 * The linker must resolve its address.
 * - If n_value is zero: the symbol is undefined and must be found elsewhere
 * - If n_value is non-zero: it's a common symbol (e.g., uninitialized global variable), 
 *   and the linker reserves that amount of space
 */
#define N_UNDF 0x0

/**
 * @brief Absolute symbol (0x2)
 * 
 * Its value is absolute, not relative to any section.
 * Common for constants, symbols that don't reside in memory segments.
 * Not relocated by the linker.
 */
#define N_ABS  0x2

/**
 * @brief Text segment symbol (0x4)
 * 
 * Refers to code (functions or instructions).
 * Address is relative to the .text segment.
 * Typically used for function names like main, _start.
 */
#define N_TEXT 0x4

/**
 * @brief Data segment symbol (0x6)
 * 
 * Refers to initialized global/static variables.
 * Address is relative to the .data segment.
 */
#define N_DATA 0x6

/**
 * @brief BSS (Block Started by Symbol) segment symbol (0x8)
 * 
 * Refers to uninitialized global/static variables.
 * Memory is allocated but not initialized in the binary.
 * BSS doesn't take space in the file, but is allocated at runtime.
 */
#define N_BSS  0x8

/**
 * @brief Zero-page relocation symbol (0xA)
 * 
 * Rare and specific to some a.out variants (like old PDP or custom platforms).
 * May refer to symbols in zero page or relocations that use page-zero addressing.
 * Typically not seen in modern binaries.
 */
#define N_ZREL 0xA

/**
 * @brief Filename (debugging) symbol (0x1F)
 * 
 * Used by debuggers to indicate the start of a source file.
 * Contains the file name in the string table.
 * Only appears in debugging builds (especially STABS-enabled a.outs).
 */
#define N_FN   0x1f

// Symbol binding definitions
#define N_EXT  0x1     // External symbol

// Memory layout constants
#define TEXT_START 00000  // Start of text segment - start at address 0
#define DATA_START(text_size) (TEXT_START + (text_size))  // Start of data segment

// a.out header structure
typedef struct {
    uint16_t a_magic;      // Magic number
    uint16_t a_text;       // Size of text segment
    uint16_t a_data;       // Size of data segment
    uint16_t a_bss;        // Size of bss segment
    uint16_t a_syms;       // Size of symbol table
    uint16_t a_entry;      // Entry point
    uint16_t a_zp;         // Zero page size
    uint16_t a_flag;       // Flags
} aout_header_t;

// Renamed to avoid conflict with system nlist
struct aout_nlist {
    union {
        char* n_name;  // Symbol name
        uint32_t n_strx;  // String table index
    } n_un;
    uint8_t n_type;    // Type flag
    uint8_t n_other;   // Other flags
    uint16_t n_desc;   // Description field
    uint32_t n_value;  // Symbol value
};

// nlist entry structure (our simplified version)
typedef struct {
    const char* name;      // Symbol name
    uint8_t type;          // Symbol type
    uint8_t other;         // Other information
    uint16_t desc;         // Symbol descriptor
    uint16_t value;        // Symbol value
} nlist_entry_t;

// Function declarations
bool nlist_parse_file(const char* filename, struct aout_nlist** entries, size_t* count, bool dump_code);
void nlist_free_entries(struct aout_nlist* entries, size_t count);

#endif /* NLIST_H */ 