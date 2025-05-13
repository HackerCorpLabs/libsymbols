#include "symbols.h"
#include "stabs.h"
#include "aout.h"
#include "mapfile.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <limits.h>

// Comparison function for qsort and bsearch
int compare_entries_by_address(const void *a, const void *b)
{
    const symbol_entry_t *entry_a = (const symbol_entry_t *)a;
    const symbol_entry_t *entry_b = (const symbol_entry_t *)b;
    return entry_a->address - entry_b->address;
}

// Helper function to map STABS type to symbol type
static symbol_type_t map_stabs_type(int type_code)
{
    switch (type_code)
    {
    case N_FUN:
        return SYMBOL_TYPE_FUNCTION;
    case N_GSYM:
    case N_LSYM:
        return SYMBOL_TYPE_VARIABLE;
    case N_SO:
        return SYMBOL_TYPE_FILE;
    case N_SLINE:
        return SYMBOL_TYPE_LINE;
    default:
        return SYMBOL_TYPE_UNKNOWN;
    }
}

// Helper function to map nlist type to symbol type
static symbol_type_t map_nlist_type(int type)
{
    switch (type & 0x1e)
    { // Mask out N_EXT bit
    case N_TEXT:
        return SYMBOL_TYPE_FUNCTION;
    case N_DATA:
    case N_BSS:
        return SYMBOL_TYPE_VARIABLE;
    default:
        return SYMBOL_TYPE_UNKNOWN;
    }
}

// Create a new symbol table
symbol_table_t *symbols_create(void)
{
    symbol_table_t *table = malloc(sizeof(symbol_table_t));
    if (!table)
        return NULL;

    table->capacity = 16;
    table->count = 0;
    table->entries = malloc(table->capacity * sizeof(symbol_entry_t));
    if (!table->entries)
    {
        free(table);
        return NULL;
    }

    return table;
}

// Free a symbol table and its contents
void symbols_free(symbol_table_t *table)
{
    if (!table)
        return;

    // Free all entries
    for (size_t i = 0; i < table->count; i++)
    {
        symbol_entry_t *entry = &table->entries[i];
        if (entry->owns_strings)
        {
            if (entry->name)
                free((void *)entry->name);
            if (entry->filename)
                free((void *)entry->filename);
        }
    }

    // Free the entries array
    free(table->entries);
    free(table);
}

/// @brief Adds a symbol to the symbol table that marks the beginning of a source file
/// @param table Pointer to the symbol table
/// @param filename Name of the source file
/// @param is_start True if this is the start of a source file, false if it is the end
/// @return True if the symbol was added, false if it already exists
bool add_file_start_symbol(symbol_table_t *table, const char *filename, bool is_start)
{
    if (!table)
        return false;

    // First check if we already have this symbol file registered
    if ((filename && is_start))
    {
        for (size_t i = 0; i < table->count; i++)
        {
            symbol_entry_t *existing = &table->entries[i];

            // compare strings
            if (strcmp(existing->filename, filename) == 0)
            {
                return false;
            }
        }
    }

    // Add a N_SO symbol to tell that source files begins here
    symbol_entry_t entry = {
        .filename = filename,
        .name = NULL,
        .line = 0,
        .address = 0,
        .type = SYMBOL_TYPE_FILE,
        .owns_strings = false};

    return symbols_add_entry(table, entry.filename, entry.name, entry.line, entry.address, entry.type);
}

// Add a new entry to the symbol table or update existing one
bool symbols_add_entry(symbol_table_t *table, const char *filename, const char *name,
                       int line, uint16_t address, symbol_type_t type)
{
    if (!table)
        return false;

    for (size_t i = 0; i < table->count; i++)
    {
        symbol_entry_t *existing = &table->entries[i];
        if ((existing->address == address) && (existing->type == type))
        {
            // Found existing symbol, update missing information
            if (filename && !existing->filename)
            {
                existing->filename = strdup(filename);
                if (!existing->filename)
                    return false;
            }
            if (name && !existing->name)
            {
                existing->name = strdup(name);
                if (!existing->name)
                {
                    if (filename && !existing->filename)
                    {
                        free((void *)existing->filename);
                    }
                    return false;
                }
            }
            if (line > 0 && existing->line == 0)
            {
                existing->line = line;
            }
            if (type != SYMBOL_TYPE_UNKNOWN && existing->type == SYMBOL_TYPE_UNKNOWN)
            {
                existing->type = type;
            }
            return true; // Successfully updated existing symbol
        }
    }

    // No existing symbol found, add new entry
    if (table->count >= table->capacity)
    {
        size_t new_capacity = table->capacity * 2;
        symbol_entry_t *new_entries = realloc(table->entries,
                                              new_capacity * sizeof(symbol_entry_t));
        if (!new_entries)
            return false;
        table->entries = new_entries;
        table->capacity = new_capacity;
    }

    // Initialize new entry
    symbol_entry_t *entry = &table->entries[table->count];
    entry->owns_strings = true;

    if (filename)
    {
        entry->filename = strdup(filename);
        if (!entry->filename)
            return false;
    }
    else
    {
        entry->filename = NULL;
    }

    if (name)
    {
        entry->name = strdup(name);
        if (!entry->name)
        {
            free((void *)entry->filename);
            return false;
        }
    }
    else
    {
        entry->name = NULL;
    }

    entry->line = line;
    entry->address = address;
    entry->type = type;

    table->count++;
    return true;
}

// Load symbols from a STABS .s file
bool symbols_load_stabs(symbol_table_t *table, const char *filename)
{
    if (!table || !filename)
        return false;

    stab_entry_t *entries = NULL;
    size_t count = 0;

    if (!stabs_parse_file(filename, &entries, &count))
    {
        return false;
    }

    // Add a symbol to tell that source files begins here
    bool start_symbol_added = add_file_start_symbol(table, entries[0].filename, true);

    bool success = true;
    for (size_t i = 0; i < count; i++)
    {
        symbol_entry_t entry = {
            .filename = entries[i].filename,
            .name = entries[i].name,
            .line = entries[i].line,
            .address = entries[i].value,
            .type = map_stabs_type(entries[i].type_code),
            .owns_strings = false};

        if (!symbols_add_entry(table, entry.filename, entry.name,
                               entry.line, entry.address, entry.type))
        {
            success = false;
            break;
        }
    }

    if (start_symbol_added)
    {
        // add ending symbol
        add_file_start_symbol(table, "", false);
    }

    stabs_free_entries(entries, count);
    return success;
}

// Load symbols from an a.out file
bool symbols_load_aout(symbol_table_t *table, const char *filename)
{
    if (!table || !filename)
        return false;

    aout_entry_t *entries = NULL;
    size_t count = 0;

    if (!aout_parse_file(filename, &entries, &count))
    {
        return false;
    }

    // Create filanme with .s ending instead of .out
    size_t fname_len = strlen(filename);
    char *filename_s = malloc(fname_len + 4);

    // replace .out with .smake all
    strncpy(filename_s, filename, strlen(filename) - 4);
    filename_s[strlen(filename) - 4] = '\0';
    strcat(filename_s, ".s");

    // Add a symbol to tell that source files begins here
    bool start_symbol_added = add_file_start_symbol(table, filename_s, true);
    free(filename_s);

    bool success = true;
    for (int i = 0; i < count; i++)
    {

        // Create symbol entry
        symbol_entry_t entry = {
            .filename = NULL,
            .name = entries[i].name,
            .line = 0,
            .address = entries[i].value,
            .type = map_nlist_type(entries[i].type),
            .owns_strings = false};

        if (!symbols_add_entry(table, entry.filename, entry.name,
                               entry.line, entry.address, entry.type))
        {
            
            success = false;
            break;
        }
        else
        {
          char s_name[100];
          if (entry.name)
          {
            strcpy(s_name, entry.name);
          }
          else
          {
            strcpy(s_name, "(null)");
          }


            printf("[%d] Added symbol: %s at %06o, Type 0x%04x '%s'", i, s_name, entry.address, entries[i].type, get_symbol_type(entries[i].type));
            if (entries[i].desc>0)
            {
                printf(", desc: %d (%s)", entries[i].desc, get_symbol_desc(entries[i].type));
            }
            printf("\n");

        }
    }

    if (start_symbol_added)
    {
        // add ending symbol
        add_file_start_symbol(table, "", false);
    }

    aout_free_entries(entries, count);
    return success;
}

// Load symbols from a map file
bool symbols_load_map(symbol_table_t *table, const char *filename)
{
    if (!table || !filename)
        return false;

    map_entry_t *entries = NULL;
    size_t count = 0;

    if (!mapfile_parse_file(filename, &entries, &count))
    {
        return false;
    }

    // Add a symbol to tell that source files begins here (use the .s file name)
    bool start_symbol_added = add_file_start_symbol(table, entries[0].filename, true);

    bool success = true;
    for (size_t i = 0; i < count; i++)
    {
        symbol_entry_t entry = {
            .filename = entries[i].filename,
            .name = NULL,
            .line = entries[i].line,
            .address = entries[i].address,
            .type = SYMBOL_TYPE_LINE,
            .owns_strings = false};

        if (!symbols_add_entry(table, entry.filename, entry.name,
                               entry.line, entry.address, entry.type))
        {
            success = false;
            break;
        }
    }

    mapfile_free_entries(entries, count);

    if (start_symbol_added)
    {
        // add ending symbol
        add_file_start_symbol(table, "", false);
    }

    return success;
}

// Look up a symbol by address
const symbol_entry_t *symbols_lookup_by_address(const symbol_table_t *table, uint16_t address)
{
    if (!table || table->count == 0)
        return NULL;

    // Create a temporary entry for searching
    symbol_entry_t key = {.address = address};

    // Use binary search to find the entry
    symbol_entry_t *result = bsearch(&key, table->entries, table->count,
                                     sizeof(symbol_entry_t), compare_entries_by_address);

    return result;
}

// Dump all symbols to stdout for debugging
void symbols_dump_all(const symbol_table_t *table)
{
    if (!table)
        return;

    printf("Symbol Table (%zu entries):\n", table->count);
    printf("----------------------------\n");

    for (size_t i = 0; i < table->count; i++)
    {
        const symbol_entry_t *entry = &table->entries[i];
        printf("Entry %zu:\n", i);
        printf("  Name: %s\n", entry->name ? entry->name : "(none)");
        printf("  Type: %d\n", entry->type);
        printf("  File: %s\n", entry->filename ? entry->filename : "(none)");
        printf("  Line: %d\n", entry->line);
        printf("  Address: %06o\n", entry->address);
        printf("\n");
    }
}

// Find address for a source location
bool symbols_find_address(const symbol_table_t *table, const char *filename, uint16_t *address, int line)
{
    if (!table || !filename)
        return 0;

    // First, find the file entry
    const symbol_entry_t *file_entry = NULL;
    for (size_t i = 0; i < table->count; i++)
    {
        if (table->entries[i].type == SYMBOL_TYPE_FILE &&
            table->entries[i].filename != NULL &&
            strcmp(table->entries[i].filename, filename) == 0)
        {
            file_entry = &table->entries[i];
            break;
        }
    }
    if (!file_entry)
        return false;

    // Then find the closest line number entry
    uint16_t closest_address = 0;
    int closest_line_diff = INT_MAX;

    for (size_t i = 0; i < table->count; i++)
    {
        if (table->entries[i].type == SYMBOL_TYPE_LINE &&
            strcmp(table->entries[i].filename, filename) == 0)
        {
            int line_diff = abs(table->entries[i].line - line);
            if (line_diff < closest_line_diff)
            {
                closest_line_diff = line_diff;
                closest_address = table->entries[i].address;
            }
        }
    }

    *address = closest_address;
    ;
    return true;
}

// Get source file for an address
const char *symbols_get_file(const symbol_table_t *table, uint16_t address)
{
    if (!table)
        return NULL;

    // Find the closest line number entry
    const symbol_entry_t *closest = NULL;
    uint16_t closest_diff = UINT16_MAX;

    for (size_t i = 0; i < table->count; i++)
    {
        if (table->entries[i].type == SYMBOL_TYPE_LINE)
        {
            uint16_t diff = abs(table->entries[i].address - address);
            if (diff < closest_diff)
            {
                closest_diff = diff;
                closest = &table->entries[i];
            }
        }
    }

    return closest ? closest->filename : NULL;
}

// Get line number for an address
int symbols_get_line(const symbol_table_t *table, uint16_t address)
{
    if (!table)
        return 0;

    // Find the closest line number entry
    const symbol_entry_t *closest = NULL;
    uint16_t closest_diff = UINT16_MAX;

    for (size_t i = 0; i < table->count; i++)
    {
        if (table->entries[i].type == SYMBOL_TYPE_LINE)
        {
            uint16_t diff = abs(table->entries[i].address - address);
            if (diff < closest_diff)
            {
                closest_diff = diff;
                closest = &table->entries[i];
            }
        }
    }

    return closest ? closest->line : 0;
}

// Check if an entry represents a line number
bool symbols_is_line_entry(const symbol_entry_t *entry)
{
    return entry && entry->type == SYMBOL_TYPE_LINE;
}

/// @brief Finds the memory address of the next source code line for stepping through code
/// @param table Pointer to the symbol table containing line number to address mappings
/// @param current_address The current memory address to find the next line from
/// @return The memory address of the next line in the same source file, or 0 if no next line exists
uint16_t symbols_get_next_line_address(const symbol_table_t *table, uint16_t current_address)
{
    if (!table)
        return 0;

    // Find the current line entry
    const symbol_entry_t *current = NULL;
    uint16_t closest_diff = UINT16_MAX;

    for (size_t i = 0; i < table->count; i++)
    {
        if (table->entries[i].type == SYMBOL_TYPE_LINE)
        {
            uint16_t diff = abs(table->entries[i].address - current_address);
            if (diff < closest_diff)
            {
                closest_diff = diff;
                current = &table->entries[i];
            }
        }
    }

    if (!current)
        return 0;

    // Find the next line entry in the same file
    const symbol_entry_t *next = NULL;
    uint16_t next_address = UINT16_MAX;

    for (size_t i = 0; i < table->count; i++)
    {
        if (table->entries[i].type == SYMBOL_TYPE_LINE &&
            strcmp(table->entries[i].filename, current->filename) == 0 &&
            table->entries[i].line > current->line &&
            table->entries[i].address < next_address)
        {
            next = &table->entries[i];
            next_address = table->entries[i].address;
        }
    }

    return next ? next->address : 0;
}

// Load binary code from a.out file
bool symbols_load_binary(const char *filename, binary_info_t *info)
{
    if (!filename || !info)
        return false;

    FILE *file = fopen(filename, "rb");
    if (!file)
        return false;

    // Read a.out header
    aout_header_t header;
    for (int i = 0; i < sizeof(header) / 2; i++) {
        uint16_t* field = ((uint16_t*)&header) + i;
        if (fread(field, 2, 1, file) != 1) {
            fclose(file);
            return false;
        }
    }

    // Initialize binary info
    info->segment_count = 0;
    info->segments = NULL;
    info->entry_point = header.a_entry;

    // Allocate segments array
    info->segments = malloc(2 * sizeof(memory_segment_t)); // Text and data segments
    if (!info->segments)
    {
        fclose(file);
        return false;
    }

    // Skip zero page if present (like in aout.c)
    fseek(file, 16 + header.a_zp * 2, SEEK_SET);

    // Load text segment
    info->segments[0].start_address = 0; // TEXT_START in memory
    info->segments[0].size = header.a_text;
    info->segments[0].is_text = true;
    info->segments[0].data = malloc(header.a_text * 2); // * 2 because we need bytes not words
    if (!info->segments[0].data)
    {
        free(info->segments);
        fclose(file);
        return false;
    }

    // Read text segment word by word and convert to bytes
    for (size_t i = 0; i < header.a_text; i++) {
        uint16_t word;
        if (fread(&word, 2, 1, file) != 1) {
            free(info->segments[0].data);
            free(info->segments);
            fclose(file);
            return false;
        }
        
        // Store in byte array (little endian)
        info->segments[0].data[i*2] = word & 0xFF;
        info->segments[0].data[i*2+1] = (word >> 8) & 0xFF;
    }

    // Load data segment - properly located after text
    info->segments[1].start_address = header.a_text; // DATA_START is text_size in aout
    info->segments[1].size = header.a_data;
    info->segments[1].is_text = false;
    info->segments[1].data = malloc(header.a_data * 2); // * 2 because we need bytes not words
    if (!info->segments[1].data)
    {
        free(info->segments[0].data);
        free(info->segments);
        fclose(file);
        return false;
    }

    // Read data segment word by word and convert to bytes
    for (size_t i = 0; i < header.a_data; i++) {
        uint16_t word;
        if (fread(&word, 2, 1, file) != 1) {
            free(info->segments[1].data);
            free(info->segments[0].data);
            free(info->segments);
            fclose(file);
            return false;
        }
        
        // Store in byte array (little endian)
        info->segments[1].data[i*2] = word & 0xFF;
        info->segments[1].data[i*2+1] = (word >> 8) & 0xFF;
    }

    info->segment_count = 2;
    fclose(file);
    return true;
}

// Free binary loading information
void symbols_free_binary(binary_info_t *info)
{
    if (!info)
        return;

    for (size_t i = 0; i < info->segment_count; i++)
    {
        free(info->segments[i].data);
    }
    free(info->segments);
    info->segments = NULL;
    info->segment_count = 0;
    info->entry_point = 0;
}

// Get memory segment containing an address
const memory_segment_t *symbols_get_segment(const binary_info_t *info, uint16_t address)
{
    if (!info)
        return NULL;

    for (size_t i = 0; i < info->segment_count; i++)
    {
        if (address >= info->segments[i].start_address &&
            address < info->segments[i].start_address + info->segments[i].size)
        {
            return &info->segments[i];
        }
    }
    return NULL;
}

// Get entry point address
uint16_t symbols_get_entry_point(const binary_info_t *info)
{
    return info ? info->entry_point : 0;
}