#ifndef MAPFILE_H
#define MAPFILE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Map entry structure
typedef struct {
    const char* filename;  // Source file name
    int line;             // Line number
    uint16_t address;     // Memory address
} map_entry_t;

// Function declarations
bool mapfile_parse_file(const char* filename, map_entry_t** entries, size_t* count);
void mapfile_free_entries(map_entry_t* entries, size_t count);

#endif /* MAPFILE_H */ 
