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
#define N_UNDF 0x0     // Undefined
#define N_ABS  0x2     // Absolute
#define N_TEXT 0x4     // Text symbol
#define N_DATA 0x6     // Data symbol
#define N_BSS  0x8     // BSS symbol
#define N_ZREL 0xA     // Zero page relocation
#define N_FN   0x1f    // File name symbol

// Symbol binding definitions
#define N_EXT  0x1     // External symbol

// Memory layout constants
#define TEXT_START 01000  // Start of text segment
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