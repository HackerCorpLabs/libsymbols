#include "nlist.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

// Helper function to read a 16-bit value in little-endian
/*
static uint16_t read_le16(const uint8_t* data) {
    return (uint16_t)data[0] | ((uint16_t)data[1] << 8);
}
*/
bool nlist_parse_file(const char* filename, struct aout_nlist** entries, size_t* count, bool dump_code) {
    if (!filename || !entries || !count) return false;

    FILE* file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "Failed to open file: %s\n", strerror(errno));
        return false;
    }

    // Read a.out header
    aout_header_t header;
    if (fread(&header, sizeof(header), 1, file) != 1) {
        fclose(file);
        return false;
    }

    // Verify magic number
    if (header.a_magic != A_MAGIC1 && 
        header.a_magic != A_MAGIC2 && 
        header.a_magic != A_MAGIC3) {
        fprintf(stderr, "Invalid a.out magic number: 0%o\n", header.a_magic);
        fclose(file);
        return false;
    }

    printf("Debug: text=%u data=%u bss=%u syms=%u\n",
           header.a_text, header.a_data, header.a_bss, header.a_syms);

    // Calculate symbol table offset
    long sym_offset = 16 +          // Header size
        (header.a_zp * 2) +         // Zero page size
        (header.a_text * 2) +       // Text segment
        (header.a_data * 2) +       // Data segment
        (header.a_zp * 2) +         // Zero page relocation
        (header.a_text * 2) +       // Text relocation
        (header.a_data * 2);        // Data relocation

    // If we're just loading symbols, seek directly to the symbol table
    if (!dump_code) {
        if (fseek(file, sym_offset, SEEK_SET) != 0) {
            fprintf(stderr, "Failed to seek to symbol table\n");
            fclose(file);
            return false;
        }
    } else {
        // For code dumping, we need to read the text and data segments
        // Skip zero page if present
        if (fseek(file, 16 + header.a_zp * 2, SEEK_SET) != 0) {
            fprintf(stderr, "Failed to skip zero page\n");
            fclose(file);
            return false;
        }

        // Read text segment
        printf("Text segment (%u words):\n", header.a_text);
        for (uint16_t i = 0; i < header.a_text; i++) {
            uint16_t word;
            if (fread(&word, sizeof(word), 1, file) != 1) {
                fprintf(stderr, "Failed to read text segment\n");
                fclose(file);
                return false;
            }
            printf("  %06o: %06o\n", TEXT_START + i, word);
        }

        // Read data segment
        printf("Data segment (%u words):\n", header.a_data);
        for (uint16_t i = 0; i < header.a_data; i++) {
            uint16_t word;
            if (fread(&word, sizeof(word), 1, file) != 1) {
                fprintf(stderr, "Failed to read data segment\n");
                fclose(file);
                return false;
            }
            printf("  %06o: %06o\n", DATA_START(header.a_text) + i, word);
        }

        // Skip relocation sections
        if (fseek(file, (header.a_zp + header.a_text + header.a_data) * 2, SEEK_CUR) != 0) {
            fprintf(stderr, "Failed to skip relocation sections\n");
            fclose(file);
            return false;
        }
    }

    // Now we're at the symbol table
    // Calculate number of symbol entries
    size_t num_symbols = (header.a_syms * 2) / sizeof(struct aout_nlist);
    if (num_symbols == 0) {
        fclose(file);
        return true;  // No symbols is not an error
    }

    // Allocate entries array
    *entries = malloc(num_symbols * sizeof(struct aout_nlist));
    if (!*entries) {
        fclose(file);
        return false;
    }

    // Remember current position for string table
    long str_table_pos = ftell(file) + (header.a_syms * 2);

    // Read symbol table
    for (size_t i = 0; i < num_symbols; i++) {
        struct aout_nlist* entry = &(*entries)[i];
        
        // Read nlist entry
        if (fread(&entry->n_un.n_strx, sizeof(entry->n_un.n_strx), 1, file) != 1 ||
            fread(&entry->n_type, sizeof(entry->n_type), 1, file) != 1 ||
            fread(&entry->n_other, sizeof(entry->n_other), 1, file) != 1 ||
            fread(&entry->n_value, sizeof(entry->n_value), 1, file) != 1) {
            free(*entries);
            fclose(file);
            return false;
        }

        // Save current position
        long cur_pos = ftell(file);

        // Read symbol name from string table
        fseek(file, str_table_pos + entry->n_un.n_strx, SEEK_SET);
        char name[64] = {0};
        fgets(name, sizeof(name), file);
        entry->n_un.n_name = strdup(name);
        
        // Restore position for next symbol
        fseek(file, cur_pos, SEEK_SET);
    }

    *count = num_symbols;
    fclose(file);
    return true;
}

void nlist_free_entries(struct aout_nlist* entries, size_t count) {
    if (!entries) return;

    // Free string table (stored in first entry's name)
    if (count > 0 && entries[0].n_un.n_name) {
        free(entries[0].n_un.n_name);
    }

    free(entries);
} 