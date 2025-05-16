#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "symbols.h"
#include "aout.h"

// Reference doc 2.11 BSD: https://www.retro11.de/ouxr/211bsd/usr/man/cat5/a.out.0.html

/*

+---------------------------------------+--------------------------------------+-----------------+-----------------------------------------------+
|  SEGMENT                              | CONTENT                              | Properties      | Purpose                                       |
+---------------------------------------+--------------------------------------+-----------------+-----------------------------------------------+
|  A.out header                         | Header information                   | Read-only       | Program identification, load address          |
+---------------------------------------+--------------------------------------+-----------------+-----------------------------------------------+
|  (Zero-page)                          | Reserved area before actual code     | N/A             | May be used to catch NULL pointer dereference |
+---------------------------------------+--------------------------------------+-----------------+-----------------------------------------------+
|  Text                                 | Executable machine instructions      | Read-only       | Memory sharing, security, efficiency          |
+---------------------------------------+--------------------------------------+-----------------+-----------------------------------------------+
|  Data                                 | Initialized global/static variables  | Read/Write      | Runtime data modification                     |
+---------------------------------------+--------------------------------------+-----------------+-----------------------------------------------+
|  Zero-page relocation                 | Relocation info for zero-page refs   | Read-only       | Fixes absolute addresses in zero-page region  |
+---------------------------------------+--------------------------------------+-----------------+-----------------------------------------------+
|  Text relocation                      | Relocation entries for TEXT segment  | Read-only       | Fixes addresses in code referencing data/syms |
+---------------------------------------+--------------------------------------+-----------------+-----------------------------------------------+
|  Data relocation                      | Relocation entries for DATA segment  | Read-only       | Fixes addresses in data referencing symbols   |
+---------------------------------------+--------------------------------------+-----------------+-----------------------------------------------+
|  Symbol table                         | Symbol names and their addresses     | Read-only       | Used for debugging, linking, symbol lookup    |
+---------------------------------------+--------------------------------------+-----------------+-----------------------------------------------+
|  String table                         | Names for symbols (longer than 8 ch) | Read-only       | Supports long symbol names in symbol table    |
+---------------------------------------+--------------------------------------+-----------------+-----------------------------------------------+


*/

// Reads a 16-bit word from file in little-endian order
static int read_word(FILE *f, uint16_t *out_word)
{
    int byte1 = fgetc(f);
    int byte2 = fgetc(f);
    if (byte1 == EOF || byte2 == EOF)
        return 0;

    *out_word = ((byte2 & 0xFF) << 8) | (byte1 & 0xFF);
    return 1;
}

// Swaps a little-endian word to big-endian for memory storage
/* NOT USED
static uint16_t to_big_endian(uint16_t word)
{
    return (word >> 8) | (word << 8);
}
*/

/// @brief Magic number to string
/// @param magic
/// @return
const char *magic2str(uint16_t magic)
{
    switch (magic)
    {
    case 0407:
        return "normal";
    case 0410:
        return "read-only text";
    case 0411:
        return "separated I&D";
    case 0405:
        return "read-only shareable";
    case 0430:
        return "auto-overlay (nonseparate)";
    case 0431:
        return "auto-overlay (separate)";
    default:
        return "Unknown magic";
    }
}

const char *get_symbol_type(uint8_t type)
{
    static char buf[100];
    // Keep the original type for comparison
    uint8_t original_type = type;
    if (type & N_EXT)
    {
        strcpy(buf, "EXTERNAL ");
    }
    else
    {
        buf[0] = '\0';
    }

    // Use original_type for the switch statement
    switch (original_type & ~N_EXT)
    {
    case N_UNDF:
        strcat(buf, "UNDEFINED");
        break;
    case N_ABS:
        strcat(buf, "ABSOLUTE");
        break;
    case N_TEXT:
        strcat(buf, "TEXT");
        break;
    case N_DATA:
        strcat(buf, "DATA");
        break;
    case N_BSS:
        strcat(buf, "BSS");
        break;
    case N_ZREL:
        strcat(buf, "ZREL");
        break;
    case N_FN:
        strcat(buf, "FN");
        break;
    case N_GSYM:
        strcat(buf, "GSYM");
        break;
    case N_FNAME:
        strcat(buf, "FNAME");
        break;
    case N_FUN:
        strcat(buf, "FUN");
        break;
    case N_STSYM:
        strcat(buf, "STSYM");
        break;
    case N_LCSYM:
        strcat(buf, "LCSYM");
        break;
    case N_MAIN:
        strcat(buf, "MAIN");
        break;
    case N_ROSYM:
        strcat(buf, "ROSYM");
        break;
    case N_PC:
        strcat(buf, "PC");
        break;
    case N_NSYMS:
        strcat(buf, "NSYMS");
        break;
    case N_NOMAP:
        strcat(buf, "NOMAP");
        break;
    case N_MAC_DEFINE:
        strcat(buf, "MAC_DEFINE");
        break;
    case N_OBJ:
        strcat(buf, "OBJ");
        break;
    case N_MAC_UNDEF:
        strcat(buf, "MAC_UNDEF");
        break;
    case N_OPT:
        strcat(buf, "OPT");
        break;
    case N_RSYM:
        strcat(buf, "RSYM");
        break;
    case N_M2C:
        strcat(buf, "M2C");
        break;
    case N_SLINE:
        strcat(buf, "SLINE");
        break;
    case N_DSLINE:
        strcat(buf, "DSLINE");
        break;
    case N_BSLINE: // Also N_BROWS
        strcat(buf, "BSLINE");
        break;
    case N_DEFD:
        strcat(buf, "DEFD");
        break;
    case N_FLINE:
        strcat(buf, "FLINE");
        break;
    case N_EHDECL: // Also N_MOD2
        strcat(buf, "EHDECL");
        break;
    case N_CATCH:
        strcat(buf, "CATCH");
        break;
    case N_SSYM:
        strcat(buf, "SSYM");
        break;
    case N_ENDM:
        strcat(buf, "ENDM");
        break;
    case N_SO:
        strcat(buf, "SO");
        break;
    case N_LSYM:
        strcat(buf, "LSYM");
        break;
    case N_BINCL:
        strcat(buf, "BINCL");
        break;
    case N_SOL:
        strcat(buf, "SOL");
        break;
    case N_PSYM:
        strcat(buf, "PSYM");
        break;
    case N_EINCL:
        strcat(buf, "EINCL");
        break;
    case N_ENTRY:
        strcat(buf, "ENTRY");
        break;
    case N_LBRAC:
        strcat(buf, "LBRAC");
        break;
    case N_EXCL:
        strcat(buf, "EXCL");
        break;
    case N_SCOPE:
        strcat(buf, "SCOPE");
        break;
    case N_RBRAC:
        strcat(buf, "RBRAC");
        break;
    case N_BCOMM:
        strcat(buf, "BCOMM");
        break;
    case N_ECOMM:
        strcat(buf, "ECOMM");
        break;
    case N_ECOML:
        strcat(buf, "ECOML");
        break;
    case N_WITH:
        strcat(buf, "WITH");
        break;
    case N_NBTEXT:
        strcat(buf, "NBTEXT");
        break;
    case N_NBDATA:
        strcat(buf, "NBDATA");
        break;
    case N_NBBSS:
        strcat(buf, "NBBSS");
        break;
    case N_NBSTS:
        strcat(buf, "NBSTS");
        break;
    case N_NBLCS:
        strcat(buf, "NBLCS");
        break;
    default:
        strcat(buf, "UNKNOWN");
        break;
    }
    return buf;
}


/// @brief Some types have a description field  
/// @param type 
/// @return 
const char *get_symbol_desc(uint8_t type)
{
    switch(type & ~N_EXT)
    {
    case N_SLINE:
        return "Source line number";
    case N_PSYM:
        return "Register number";
    case N_RSYM:
        return "Register number";
    case N_LSYM:
        return "Register number (if register variable)";
    case N_LBRAC:
        return "Nesting level";
    default:
        return "Not used";
    }
}

void load_symbols_with_string_table(FILE *f, long sym_offset, uint32_t num_bytes_syms, bool verbose)
{
    // Seek to symbol table
    fseek(f, sym_offset, SEEK_SET);

    if (verbose)
    {
        // Read and print symbols
        printf("%-50s %-12s %-10s %s\n", "NAME", "TYPE","N_TYPE", "VALUE");
        printf("----------------------------------------\n");
    }

    // Calculate number of symbol entries
    int num_symbols = (num_bytes_syms * 2) / sizeof(aout_nlist_t);

    // Remember current position for string table
    long str_table_pos = sym_offset + (num_bytes_syms * 2);

    for (int i = 0; i < num_symbols; i++)
    {
        aout_nlist_t sym;
        if (fread(&sym, sizeof(sym), 1, f) != 1)
            break;

        // Save current position
        long cur_pos = ftell(f);

        // Read symbol name from string table
        fseek(f, str_table_pos + sym.n_strx, SEEK_SET);
        char name[64] = {0};
        size_t bytes_read = fread(name, 1, sizeof(name) - 1, f);
        if (bytes_read > 0) {
            name[bytes_read] = '\0';  // Ensure null termination
        }

        // add_symbol((symbol_t*)&sym);

        if (verbose)
        {
            // Print symbol information|
            printf("%-50s %-12s 0x%02x %06o\n",
                   name,
                   get_symbol_type(sym.n_type),
                   sym.n_type,
                   sym.n_value);
        }

        // Restore position for next symbol
        fseek(f, cur_pos, SEEK_SET);
    }
}

/// @brief Load a.out header from file. 
/// @param f File pointer to the a.out file
/// @param header Pointer to aout_header_t structure to store the header information
/// @param verbose Print header information if true
/// @return  0 on success, -1 on error
/// @details Reads the a.out header fields from the file and stores them in the provided structure.
int load_header(FILE *f, aout_header_t *header, bool verbose)
{
    // Read and parse all 8 header fields from the file
     // Read and parse all 8 header fields from the file
    for (int i = 0; i < sizeof(*header) / 2; i++) {
        uint16_t* field = ((uint16_t*)header) + i;
        if (!read_word(f, field)) {
            fprintf(stderr, "Failed to read a.out header field %d\n", i);
            fclose(f);
            return -1;
        }
    }


    if (verbose)
    {
        // Print header information for debugging
        printf("=== Loaded a.out Header ===\n");
        printf("  Magic     : 0x%04X (%s)\n", header->a_magic, magic2str(header->a_magic));
        printf("  Text size : %u words\n", header->a_text);
        printf("  Data size : %u word\n", header->a_data);
        printf("  BSS size  : %u words (will be zero-filled if needed)\n", header->a_bss);
        printf("  Symbols   : %u bytes\n", header->a_syms);
        printf("  Entry     : 0%06o\n", header->a_entry);
        printf("  Zero Page : %u words\n", header->a_zp);
        printf("  Flags     : 0%06o\n", header->a_flag);
        printf("===========================\n");
    }
    return 0;
}

/// @brief Loads a PDP-11 a.out file and writes the text/data segments to memory.
/// @param filename The filename of the a.out file to load
/// @return -1 on error, else the entry point address
int load_aout(const char *filename, bool verbose, write_memory_callback write_memory)
{
    uint16_t dataLoadAddress;

    aout_header_t header;
    memset(&header, 0, sizeof(header));

    FILE *f = fopen(filename, "rb");
    if (!f)
    {
        perror("Failed to open file");
        return -1;
    }

    if (load_header(f, &header, verbose) != 0)
    {
        fclose(f);
        return -1;
    }

    // Skip zero page if present
    fseek(f, 16 + header.a_zp * 2, SEEK_SET);

    uint16_t memoryPtr = 0;
    // Load the text segment
    if (verbose)
        printf("Loading text segment at 0%06o (%u bytes)\n", TEXT_START, header.a_text);

    for (uint16_t i = 0; i < header.a_text; i++)
    {
        uint16_t word;
        if (!read_word(f, &word))
        {
            fprintf(stderr, "Unexpected EOF while reading text segment\n");
            break;
        }

        dataLoadAddress = TEXT_START + memoryPtr;
        // printf("Writing TEXT %06o to %06o\n", word, dataLoadAddress);

        if (write_memory)
            write_memory(dataLoadAddress, word);
        memoryPtr++;
    }

    // Load the data segment
    uint16_t data_addr = DATA_START(header.a_text);
    if (verbose)
        printf("Loading data segment at 0%06o (%u bytes)\n", data_addr, header.a_data);

    for (uint16_t i = 0; i < header.a_data; i++)
    {
        uint16_t word;
        if (!read_word(f, &word))
        {
            fprintf(stderr, "Unexpected EOF while reading data segment\n");
            break;
        }

        dataLoadAddress = data_addr + memoryPtr;
        // printf("Writing DATA %06o to %06o\n", word, dataLoadAddress);

        if (write_memory)
            write_memory(dataLoadAddress, word);

        memoryPtr++;
    }

    // Calculate symbol table offset
    long sym_offset = 16 +                  // Header size
                      (header.a_zp * 2) +   // Zero page size
                      (header.a_text * 2) + // Text segment
                      (header.a_data * 2) + // Data segment
                      (header.a_zp * 2) +   // Zero page relocation
                      (header.a_text * 2) + // Text relocation
                      (header.a_data * 2);  // Data relocation

    if (verbose)
        printf("Loading symbols(%u bytes)\n", header.a_syms);

    // Load symbols
    load_symbols_with_string_table(f, sym_offset, header.a_syms, verbose);

    fclose(f);

    if (verbose)
        printf("\n\n");
    return header.a_entry; // return entry address for execution
}

/// @brief Load a.out header and symbol table from file.
/// @param filename
/// @param entries
/// @param count
/// @return
bool aout_parse_file(const char *filename, aout_entry_t **entries, size_t *count)
{
    *count = 0;
    *entries = NULL;

    aout_header_t header;
    memset(&header, 0, sizeof(header));

    FILE *f = fopen(filename, "rb");
    if (!f)
    {
        perror("Failed to open file");
        return false;
    }    

    if (load_header(f, &header, false) != 0)
    {
        fclose(f);
        return false;
    }

    // Calculate symbol table offset
    long sym_offset = 16 +                  // Header size
                      (header.a_zp * 2) +   // Zero page size
                      (header.a_text * 2) + // Text segment
                      (header.a_data * 2) + // Data segment
                      (header.a_zp * 2) +   // Zero page relocation
                      (header.a_text * 2) + // Text relocation
                      (header.a_data * 2);  // Data relocation

    // Seek to symbol table
    fseek(f, sym_offset, SEEK_SET);

    // Calculate number of symbol entries
    int num_symbols = (header.a_syms * 2) / sizeof(aout_nlist_t);
    *count = num_symbols;

    // Remember current position for string table
    long str_table_pos = sym_offset + (header.a_syms * 2);

    // Allocate the entries array
    *entries = calloc(num_symbols, sizeof(aout_entry_t));
    if (!*entries) {
        fclose(f);
        return false;
    }
    
    aout_nlist_t nlist_sym;

    for (int i = 0; i < num_symbols; i++)
    {            
        // Read the symbol entry
        if (fread(&nlist_sym, sizeof(nlist_sym), 1, f) != 1)
            break;

        // Save current position
        long cur_pos = ftell(f);

        // Read symbol name from string table
        fseek(f, str_table_pos + nlist_sym.n_strx, SEEK_SET);
        char name[64] = {0};
        size_t bytes_read = fread(name, 1, sizeof(name) - 1, f);
        if (bytes_read > 0) {
            name[bytes_read] = '\0';  // Ensure null termination
        }

        // Initialize the entry directly in the array
        (*entries)[i].name = strdup(name);
        (*entries)[i].desc = (nlist_sym.n_type>>8) & 0xFF;
        (*entries)[i].type = (nlist_sym.n_type & 0xFF);
        (*entries)[i].value = nlist_sym.n_value;

        if (true)
        {
            // Print symbol information
            printf("%-70s %-20s 0x%02x 0x%02x %06o\n",
                   (*entries)[i].name,
                   get_symbol_type((*entries)[i].type),
                   (*entries)[i].type,
                   (*entries)[i].desc,
                   (*entries)[i].value);
        }

        // Restore position for next symbol
        fseek(f, cur_pos, SEEK_SET);
    }

    fclose(f);
    return true;
}

void aout_free_entries(aout_entry_t *entries, size_t count)
{
    if (!entries)
        return;

    for (size_t i = 0; i < count; i++)
    {
        if (entries[i].name)
        {
            free(entries[i].name);
        }
    }
    free(entries);
}