#ifndef STABS_H
#define STABS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// STABS type definitions
#define N_GSYM   0x20    // Global symbol
#define N_FNAME  0x22    // Function name
#define N_FUN    0x24    // Function
#define N_STSYM  0x26    // Static symbol
#define N_LCSYM  0x28    // Local common symbol
#define N_MAIN   0x2a    // Main function name
#define N_SO     0x64    // Source file name
#define N_SOL    0x84    // Included source file name
#define N_BINCL  0x82    // Beginning of include file
#define N_EINCL  0xa2    // End of include file
#define N_SLINE  0x44    // Line number in text segment
#define N_DSLINE 0x46    // Line number in data segment
#define N_BSLINE 0x48    // Line number in bss segment
#define N_RSYM   0x40    // Register symbol
#define N_LSYM   0x80    // Local symbol
#define N_PSYM   0xa0    // Parameter symbol
#define N_LBRAC  0xc0    // Left bracket
#define N_RBRAC  0xe0    // Right bracket

// STABS string format: "name:symbol-descriptor type-information"
typedef struct {
    const char* name;        // Symbol name
    char desc;              // Symbol descriptor
    const char* type;       // Type information
    uint16_t value;         // Symbol value (address)
    uint8_t type_code;      // STABS type code
    uint16_t line;          // Line number (for N_SLINE)
    const char* filename;   // Source file name
} stab_entry_t;

// Forward declarations
void stabs_free_entries(stab_entry_t* entries, size_t count);

// Function declarations
bool stabs_parse_file(const char* filename, stab_entry_t** entries, size_t* count);

#endif /* STABS_H */ 
