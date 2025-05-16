#ifndef AOUT_H
#define AOUT_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

// a.out format definitions
#define A_MAGIC1 0407    // Normal executable
#define A_MAGIC2 0410    // Read-only text
#define A_MAGIC3 0411    // Separated I&D
#define A_MAGIC4 0405    // Read-only shareable
#define A_MAGIC5 0430    // Auto-overlay (nonseparate)
#define A_MAGIC6 0431    // Auto-overlay (separate)

// Symbol type definitions
// a.out header structure from ndlib_types.h
typedef struct {
    uint16_t a_magic;   // magic number identifying valid a.out file
    uint16_t a_text;    // size of text segment (in bytes)
    uint16_t a_data;    // size of initialized data segment (in bytes)
    uint16_t a_bss;     // size of uninitialized (zero-filled) data (in bytes)
    uint16_t a_syms;    // size of symbol table (optional)
    uint16_t a_entry;   // entry point (where execution starts)
    uint16_t a_zp;      // size of zero page
    uint16_t a_flag;    // flags for relocation/symbols/etc
} aout_header_t;


/// @brief Symbol table entry structure in memory 
/// @details This structure is used to represent a symbol in memory.
/// @details The name field is 6 characters long, plus a null-terminator.
typedef struct {
    char name[7]; // +1 for null-terminator
    uint8_t type;
    uint8_t other;
    uint16_t value;
} symbol_t;

/// @brief Symbol table entry structure on disk
typedef struct {
    uint32_t n_strx;   // Offset into string table
    uint16_t n_type;   // Symbol type
    uint16_t n_value;  // Symbol value
} aout_nlist_t;


/// @brief Symbol table entry structure on disk
typedef struct {
    char *name;   // Symbol name
    uint8_t desc;   // Symbol description (source line number, register number, nesting level, etc)
    uint8_t type;   // Symbol type    
    uint16_t value;  // Symbol value
} aout_entry_t;



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

/**
 * @brief Global symbol (0x20)
 * @details Represents a global variable.
 * @see stabs.txt, Section 4.2
 */
#define N_GSYM 0x20

/**
 * @brief Function name (0x22)
 * @details Used by BSD Fortran for function names.
 * @see stabs.txt, Section 2.5
 */
#define N_FNAME 0x22

/**
 * @brief Function name or text segment variable (0x24)
 * @details Represents a function name or a static variable in the text segment.
 * @see stabs.txt, Section 2.5 (functions), Section 4.5 (statics)
 */
#define N_FUN 0x24

/**
 * @brief Data segment file-scope variable (0x26)
 * @details Represents a static variable in the data segment.
 * @see stabs.txt, Section 4.5
 */
#define N_STSYM 0x26

/**
 * @brief BSS segment file-scope variable (0x28)
 * @details Represents a static variable in the BSS segment.
 * @see stabs.txt, Section 4.5
 */
#define N_LCSYM 0x28

/**
 * @brief Name of main routine (0x2a)
 * @details Specifies the name of the main program function.
 * @see stabs.txt, Section 2.1
 */
#define N_MAIN 0x2a

/**
 * @brief Variable in .rodata section (0x2c)
 * @details Represents a static variable in a read-only data section.
 * @see stabs.txt, Section 4.5
 */
#define N_ROSYM 0x2c

/**
 * @brief Global symbol (Pascal) (0x30)
 * @details Represents a global symbol, typically for Pascal.
 * @see stabs.txt, Appendix D.1
 */
#define N_PC 0x30

/**
 * @brief Number of symbols (0x32)
 * @details Used by Ultrix V4.0 to denote the number of symbols.
 * @see stabs.txt, Appendix D.2
 */
#define N_NSYMS 0x32

/**
 * @brief No DST map (0x34)
 * @details Indicates that a symbol has no DST map (Ultrix V4.0), possibly optimized out.
 * @see stabs.txt, Appendix D.3
 */
#define N_NOMAP 0x34

/**
 * @brief Macro definition (0x36)
 * @details Contains the name and body of a #defined macro.
 * @see stabs.txt, Chapter 6
 */
#define N_MAC_DEFINE 0x36

/**
 * @brief Object file (0x38)
 * @details Used by Solaris2 to mark an object file.
 * @see stabs.txt, Appendix A.2
 */
#define N_OBJ 0x38

/**
 * @brief Macro undefinition (0x3a)
 * @details Contains the name of an #undefed macro.
 * @see stabs.txt, Chapter 6
 */
#define N_MAC_UNDEF 0x3a

/**
 * @brief Debugger options (0x3c)
 * @details Used by Solaris2 for debugger options.
 * @see stabs.txt, Appendix A.2
 */
#define N_OPT 0x3c

/**
 * @brief Register variable (0x40)
 * @details Represents a variable stored in a register.
 * @see stabs.txt, Section 4.3
 */
#define N_RSYM 0x40

/**
 * @brief Modula-2 compilation unit (0x42)
 * @details Denotes a Modula-2 compilation unit.
 * @see stabs.txt, Appendix D.4
 */
#define N_M2C 0x42

/**
 * @brief Line number in text segment (0x44)
 * @details Marks a line number in the code/text segment.
 * @see stabs.txt, Section 2.4
 */
#define N_SLINE 0x44

/**
 * @brief Line number in data segment (0x46)
 * @details Marks a line number in the data segment.
 * @see stabs.txt, Section 2.4
 */
#define N_DSLINE 0x46

/**
 * @brief Line number in bss segment (0x48)
 * @details Marks a line number in the BSS segment. Also N_BROWS for Sun source code browser.
 * @see stabs.txt, Section 2.4 and Appendix D.5
 */
#define N_BSLINE 0x48
// #define N_BROWS 0x48 // Alias for N_BSLINE (Sun source code browser)

/**
 * @brief GNU Modula2 definition module dependency (0x4a)
 * @details Indicates a dependency on a GNU Modula-2 definition module.
 * @see stabs.txt, Appendix D.6
 */
#define N_DEFD 0x4a

/**
 * @brief Function start/body/end line numbers (0x4c)
 * @details Used by Solaris2 for function line number information.
 * @see stabs.txt, Appendix A.2
 */
#define N_FLINE 0x4c

/**
 * @brief GNU C++ exception variable (0x50)
 * @details Represents a GNU C++ exception handling variable. Also N_MOD2 for Modula2 info (Ultrix V4.0).
 * @see stabs.txt, Appendix D.7 and D.8
 */
#define N_EHDECL 0x50
// #define N_MOD2 0x50 // Alias for N_EHDECL (Modula2 info for Ultrix V4.0)

/**
 * @brief GNU C++ catch clause (0x54)
 * @details Denotes a GNU C++ catch clause.
 * @see stabs.txt, Appendix D.9
 */
#define N_CATCH 0x54

/**
 * @brief Structure or union element (0x60)
 * @details Represents an element of a structure or union.
 * @see stabs.txt, Appendix D.10
 */
#define N_SSYM 0x60

/**
 * @brief Last stab for module (0x62)
 * @details Used by Solaris2 to mark the last stab for a module.
 * @see stabs.txt, Appendix A.2
 */
#define N_ENDM 0x62

/**
 * @brief Path and name of source file (0x64)
 * @details Specifies the path and name of a source file.
 * @see stabs.txt, Section 2.2
 */
#define N_SO 0x64

/**
 * @brief Stack variable or type (0x80)
 * @details Represents a variable allocated on the stack or a type definition.
 * @see stabs.txt, Section 4.1 (stack variables), Section 5.9 (types)
 */
#define N_LSYM 0x80

/**
 * @brief Beginning of an include file (Sun) (0x82)
 * @details Marks the beginning of an included file (Sun Microsystems specific).
 * @see stabs.txt, Section 2.3
 */
#define N_BINCL 0x82

/**
 * @brief Name of include file (0x84)
 * @details Specifies the name of an included file.
 * @see stabs.txt, Section 2.3
 */
#define N_SOL 0x84

/**
 * @brief Parameter variable (0xa0)
 * @details Represents a parameter to a function.
 * @see stabs.txt, Section 4.7
 */
#define N_PSYM 0xa0

/**
 * @brief End of an include file (0xa2)
 * @details Marks the end of an included file.
 * @see stabs.txt, Section 2.3
 */
#define N_EINCL 0xa2

/**
 * @brief Alternate entry point (0xa4)
 * @details Specifies an alternate entry point into a procedure or function.
 * @see stabs.txt, Section 2.8
 */
#define N_ENTRY 0xa4

/**
 * @brief Beginning of a lexical block (0xc0)
 * @details Marks the beginning of a lexical scope block.
 * @see stabs.txt, Section 2.7
 */
#define N_LBRAC 0xc0

/**
 * @brief Placeholder for a deleted include file (0xc2)
 * @details Acts as a placeholder for an include file that has been removed or optimized out.
 * @see stabs.txt, Section 2.3
 */
#define N_EXCL 0xc2

/**
 * @brief Modula2 scope information (Sun linker) (0xc4)
 * @details Used by the Sun linker for Modula-2 scope information.
 * @see stabs.txt, Appendix D.11
 */
#define N_SCOPE 0xc4

/**
 * @brief End of a lexical block (0xe0)
 * @details Marks the end of a lexical scope block.
 * @see stabs.txt, Section 2.7
 */
#define N_RBRAC 0xe0

/**
 * @brief Begin named common block (0xe2)
 * @details Marks the beginning of a named common block (e.g., in Fortran).
 * @see stabs.txt, Section 4.4
 */
#define N_BCOMM 0xe2

/**
 * @brief End named common block (0xe4)
 * @details Marks the end of a named common block.
 * @see stabs.txt, Section 4.4
 */
#define N_ECOMM 0xe4

/**
 * @brief Member of a common block (0xe8)
 * @details Represents a member variable within a common block.
 * @see stabs.txt, Section 4.4
 */
#define N_ECOML 0xe8

/**
 * @brief Pascal with statement (Solaris2) (0xea)
 * @details Represents a Pascal 'with' statement (Solaris2 specific).
 * @see stabs.txt, Appendix A.2
 */
#define N_WITH 0xea

/**
 * @brief Gould non-base registers (text) (0xf0)
 * @details Used on Gould systems for non-base registers in text section.
 * @see stabs.txt, Appendix D.12
 */
#define N_NBTEXT 0xf0

/**
 * @brief Gould non-base registers (data) (0xf2)
 * @details Used on Gould systems for non-base registers in data section.
 * @see stabs.txt, Appendix D.12
 */
#define N_NBDATA 0xf2

/**
 * @brief Gould non-base registers (BSS) (0xf4)
 * @details Used on Gould systems for non-base registers in BSS section.
 * @see stabs.txt, Appendix D.12
 */
#define N_NBBSS 0xf4

/**
 * @brief Gould non-base registers (STS) (0xf6)
 * @details Used on Gould systems for non-base registers (STS type).
 * @see stabs.txt, Appendix D.12
 */
#define N_NBSTS 0xf6

/**
 * @brief Gould non-base registers (LCS) (0xf8)
 * @details Used on Gould systems for non-base registers (LCS type).
 * @see stabs.txt, Appendix D.12
 */
#define N_NBLCS 0xf8

/// @brief External symbol
#define N_EXT  0x1     // External symbol


/// @brief Mask for type field in n_type
#define N_TYPE 0x1e      // Mask for type field in n_type


// Memory layout constants
#define TEXT_START 00000  // Start of text segment - start at address 0
#define DATA_START(text_size) (TEXT_START + (text_size))  // Start of data segment


// I need a callback function to write to memory
typedef void (*write_memory_callback)(uint16_t address, uint16_t value);

/// @brief Parse a.out file and return a list of symbols
/// @param filename 
/// @param entries 
/// @param count 
/// @return 
bool aout_parse_file(const char *filename, aout_entry_t **entries, size_t *count);

/// @brief Load a.out file into memory
/// @param filename 
/// @param verbose 
/// @param write_memory 
/// @return 
int load_aout(const char* filename, bool verbose, write_memory_callback write_memory);

/// @brief Get the type of a symbol
/// @param type 
/// @return 
const char *get_symbol_type(uint8_t type);

/// @brief Get the description of a symbol
/// @param desc 
/// @return Description of the 'desc' field for this type
const char *get_symbol_desc(uint8_t type);


/// @brief Get the symbol type from a nlist type
/// @param type
void aout_free_entries(aout_entry_t *entries, size_t count);

/// @brief Load symbols from a file
/// @param f
int load_header(FILE *f, aout_header_t *header, bool verbose);

#endif /* AOUT_H */ 
