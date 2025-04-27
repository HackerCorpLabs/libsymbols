#include "mapfile.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <stdint.h>

// Helper function to trim whitespace from a string
static char* trim_whitespace(char* str) {
    while (isspace((unsigned char)*str)) str++;
    
    if (*str == 0) return str;
    
    char* end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    
    return str;
}

bool mapfile_parse_file(const char* filename, map_entry_t** entries, size_t* count) {
    if (!filename || !entries || !count) return false;

    FILE* file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Failed to open file: %s\n", strerror(errno));
        return false;
    }

    // Initial allocation
    size_t capacity = 16;
    *entries = malloc(capacity * sizeof(map_entry_t));
    if (!*entries) {
        fclose(file);
        return false;
    }

    *count = 0;
    char line[256];

    while (fgets(line, sizeof(line), file)) {
        // Skip empty lines and comments
        char* trimmed = trim_whitespace(line);
        if (*trimmed == '\0' || *trimmed == '#') continue;

        // Parse filename:line -> address
        char* colon = strchr(trimmed, ':');
        if (!colon) continue;

        char* arrow = strstr(colon, "->");
        if (!arrow) continue;

        // Extract filename
        *colon = '\0';
        char* filename = trim_whitespace(trimmed);

        // Extract line number
        char* line_str = trim_whitespace(colon + 1);
        *arrow = '\0';
        int line_num = atoi(line_str);

        // Extract address
        char* addr_str = trim_whitespace(arrow + 2);
        uint16_t address = (uint16_t)strtoul(addr_str, NULL, 16);

        // Resize if needed
        if (*count >= capacity) {
            capacity *= 2;
            map_entry_t* new_entries = realloc(*entries, capacity * sizeof(map_entry_t));
            if (!new_entries) {
                mapfile_free_entries(*entries, *count);
                fclose(file);
                return false;
            }
            *entries = new_entries;
        }

        // Add entry
        map_entry_t* entry = &(*entries)[*count];
        entry->filename = strdup(filename);
        entry->line = line_num;
        entry->address = address;

        if (!entry->filename) {
            mapfile_free_entries(*entries, *count);
            fclose(file);
            return false;
        }

        (*count)++;
    }

    fclose(file);
    return true;
}

void mapfile_free_entries(map_entry_t* entries, size_t count) {
    if (!entries) return;

    for (size_t i = 0; i < count; i++) {
        free((void*)entries[i].filename);
    }
    free(entries);
} 