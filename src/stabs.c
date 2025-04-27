#include "stabs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Internal helper functions
static bool parse_stab_string(const char* str, stab_entry_t* entry) {
    if (!str || !entry) return false;

    // Find the colon separator
    const char* colon = strchr(str, ':');
    if (!colon) return false;

    // Extract name (everything before the colon)
    size_t name_len = colon - str;
    entry->name = strndup(str, name_len);

    // Move past the colon
    const char* type_start = colon + 1;
    if (!*type_start) return false;

    // Extract descriptor (first character after colon)
    entry->desc = *type_start;
    type_start++;

    // The rest is type information
    entry->type = strdup(type_start);

    return true;
}

static bool parse_stab_line(const char* line, stab_entry_t* entry) {
    if (!line || !entry) return false;

    // Skip leading whitespace
    while (isspace(*line)) line++;

    // Check for .stabs or .stabn directive
    if (strncmp(line, ".stabs", 6) == 0) {
        line += 6;
    } else if (strncmp(line, ".stabn", 6) == 0) {
        line += 6;
    } else {
        return false;
    }

    // Skip whitespace after directive
    while (isspace(*line)) line++;

    // Parse the string part (between quotes)
    if (*line != '"') return false;
    line++;

    const char* str_end = strchr(line, '"');
    if (!str_end) return false;

    char* stab_str = strndup(line, str_end - line);
    bool result = parse_stab_string(stab_str, entry);
    free(stab_str);

    if (!result) return false;

    // Parse the numeric fields after the string
    line = str_end + 1;
    while (isspace(*line)) line++;

    // Parse type code, other, desc, value
    int type_code, other, desc, value;
    if (sscanf(line, ",%d,%d,%d,%d", &type_code, &other, &desc, &value) != 4) {
        return false;
    }

    entry->type_code = type_code;
    entry->value = value;

    return true;
}

void stabs_free_entries(stab_entry_t* entries, size_t count) {
    if (!entries) return;

    for (size_t i = 0; i < count; i++) {
        free((void*)entries[i].name);
        free((void*)entries[i].type);
    }
    free(entries);
}

bool stabs_parse_file(const char* filename, stab_entry_t** entries, size_t* count) {
    if (!filename || !entries || !count) return false;

    FILE* file = fopen(filename, "r");
    if (!file) return false;

    // Initial allocation
    size_t capacity = 16;
    *entries = malloc(capacity * sizeof(stab_entry_t));
    if (!*entries) {
        fclose(file);
        return false;
    }

    *count = 0;
    char line[1024];
    const char* current_file = NULL;

    while (fgets(line, sizeof(line), file)) {
        stab_entry_t entry = {0};
        
        if (parse_stab_line(line, &entry)) {
            // Handle file name entries
            if (entry.type_code == N_SO) {
                current_file = entry.name;
            }
            entry.filename = current_file;

            // Resize array if needed
            if (*count >= capacity) {
                capacity *= 2;
                stab_entry_t* new_entries = realloc(*entries, capacity * sizeof(stab_entry_t));
                if (!new_entries) {
                    stabs_free_entries(*entries, *count);
                    fclose(file);
                    return false;
                }
                *entries = new_entries;
            }

            (*entries)[(*count)++] = entry;
        }
    }

    fclose(file);
    return true;
} 