/*
 * srcmap_debug.c - Parser for extended .srcmap debug info
 *
 * Parses FUNC, PARAM, LOCAL, LBRAC, RBRAC entries from the
 * linker-generated .srcmap file to build C-level debug info.
 */

#include "symbols.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

symbol_debug_info_t *
symbols_debug_info_create(void)
{
    symbol_debug_info_t *info = calloc(1, sizeof(*info));
    return info;
}

void
symbols_debug_info_free(symbol_debug_info_t *info)
{
    int i, j;

    if (!info)
        return;

    for (i = 0; i < info->function_count; i++) {
        symbol_function_t *fn = &info->functions[i];
        free(fn->name);
        for (j = 0; j < fn->variable_count; j++) {
            free(fn->variables[j].name);
            free(fn->variables[j].type_name);
        }
        free(fn->variables);
    }
    free(info->functions);
    free(info);
}

/*
 * Find (or create) a function entry by name.
 */
static symbol_function_t *
find_or_add_function(symbol_debug_info_t *info, const char *name)
{
    int i;

    for (i = 0; i < info->function_count; i++) {
        if (strcmp(info->functions[i].name, name) == 0)
            return &info->functions[i];
    }

    /* Add new function */
    if (info->function_count >= info->function_capacity) {
        int newcap = info->function_capacity ? info->function_capacity * 2 : 8;
        symbol_function_t *nf = realloc(info->functions,
                                        newcap * sizeof(*nf));
        if (!nf)
            return NULL;
        info->functions = nf;
        info->function_capacity = newcap;
    }

    symbol_function_t *fn = &info->functions[info->function_count++];
    memset(fn, 0, sizeof(*fn));
    fn->name = strdup(name);
    fn->end_address = 0xFFFF; /* sentinel until RBRAC sets it */
    return fn;
}

/*
 * Add a variable (param or local) to a function.
 */
static bool
add_variable(symbol_function_t *fn, const char *name,
             const char *type_name, int offset, bool is_parameter)
{
    if (fn->variable_count >= fn->variable_capacity) {
        int newcap = fn->variable_capacity ? fn->variable_capacity * 2 : 8;
        symbol_variable_t *nv = realloc(fn->variables,
                                        newcap * sizeof(*nv));
        if (!nv)
            return false;
        fn->variables = nv;
        fn->variable_capacity = newcap;
    }

    symbol_variable_t *v = &fn->variables[fn->variable_count++];
    v->name = strdup(name);
    v->type_name = strdup(type_name);
    v->offset = offset;
    v->is_parameter = is_parameter;
    return true;
}

/*
 * Trim leading/trailing whitespace in-place.
 */
static char *
trim(char *s)
{
    while (isspace((unsigned char)*s)) s++;
    if (*s == '\0') return s;
    char *e = s + strlen(s) - 1;
    while (e > s && isspace((unsigned char)*e)) *e-- = '\0';
    return s;
}

bool
symbols_load_srcmap_debug(symbol_debug_info_t *info, const char *filename)
{
    FILE *f;
    char line[512];

    if (!info || !filename)
        return false;

    f = fopen(filename, "r");
    if (!f)
        return false;

    while (fgets(line, sizeof(line), f)) {
        char *p = trim(line);

        /* Skip comments and empty lines */
        if (*p == '#' || *p == '\0')
            continue;

        /*
         * FUNC:name -> address
         */
        if (strncmp(p, "FUNC:", 5) == 0) {
            char *arrow = strstr(p + 5, "->");
            if (!arrow) continue;

            *arrow = '\0';
            char *fname = trim(p + 5);
            char *addr_str = trim(arrow + 2);
            uint16_t addr = (uint16_t)strtoul(addr_str, NULL, 8);

            symbol_function_t *fn = find_or_add_function(info, fname);
            if (fn)
                fn->start_address = addr;
            continue;
        }

        /*
         * PARAM:funcname:varname:type -> offset
         */
        if (strncmp(p, "PARAM:", 6) == 0) {
            /* Parse PARAM:func:name:type -> offset */
            char *rest = p + 6;
            char *c1 = strchr(rest, ':');
            if (!c1) continue;
            *c1 = '\0';
            char *funcname = rest;

            char *c2 = strchr(c1 + 1, ':');
            if (!c2) continue;
            *c2 = '\0';
            char *varname = c1 + 1;

            char *arrow = strstr(c2 + 1, "->");
            if (!arrow) continue;
            *arrow = '\0';
            char *typename_str = trim(c2 + 1);
            char *off_str = trim(arrow + 2);
            int offset = atoi(off_str);

            symbol_function_t *fn = find_or_add_function(info, funcname);
            if (fn)
                add_variable(fn, varname, typename_str, offset, true);
            continue;
        }

        /*
         * LOCAL:funcname:varname:type -> offset
         */
        if (strncmp(p, "LOCAL:", 6) == 0) {
            char *rest = p + 6;
            char *c1 = strchr(rest, ':');
            if (!c1) continue;
            *c1 = '\0';
            char *funcname = rest;

            char *c2 = strchr(c1 + 1, ':');
            if (!c2) continue;
            *c2 = '\0';
            char *varname = c1 + 1;

            char *arrow = strstr(c2 + 1, "->");
            if (!arrow) continue;
            *arrow = '\0';
            char *typename_str = trim(c2 + 1);
            char *off_str = trim(arrow + 2);
            int offset = atoi(off_str);

            symbol_function_t *fn = find_or_add_function(info, funcname);
            if (fn)
                add_variable(fn, varname, typename_str, offset, false);
            continue;
        }

        /*
         * LBRAC:funcname -> address  (scope begin - unused for now)
         */
        if (strncmp(p, "LBRAC:", 6) == 0) {
            /* Currently not used; could refine scope ranges */
            continue;
        }

        /*
         * RBRAC:funcname -> address  (scope end = function end address)
         */
        if (strncmp(p, "RBRAC:", 6) == 0) {
            char *arrow = strstr(p + 6, "->");
            if (!arrow) continue;

            *arrow = '\0';
            char *funcname = trim(p + 6);
            char *addr_str = trim(arrow + 2);
            uint16_t addr = (uint16_t)strtoul(addr_str, NULL, 8);

            symbol_function_t *fn = find_or_add_function(info, funcname);
            if (fn)
                fn->end_address = addr;
            continue;
        }

        /* Regular srcmap lines (filename:line -> address) are ignored here;
         * they are handled by the existing mapfile_parse_file() */
    }

    fclose(f);

    /* Fix up end_address: use the next function's start_address - 1
     * instead of RBRAC, because the return code (after RBRAC) is still
     * part of the function. Sort by start_address first. */
    for (int i = 0; i < info->function_count; i++) {
        /* Find the function that starts immediately after this one */
        uint16_t next_start = 0xFFFF;
        for (int j = 0; j < info->function_count; j++) {
            if (j == i) continue;
            if (info->functions[j].start_address > info->functions[i].start_address &&
                info->functions[j].start_address < next_start)
                next_start = info->functions[j].start_address;
        }
        if (next_start != 0xFFFF)
            info->functions[i].end_address = next_start - 1;
        /* else keep the existing end_address (RBRAC or 0xFFFF sentinel) */
    }

    return info->function_count > 0;
}

symbol_function_t *
symbols_find_function_at(symbol_debug_info_t *info, uint16_t address)
{
    int i;
    symbol_function_t *best = NULL;
    uint16_t best_range = 0xFFFF;

    if (!info)
        return NULL;

    for (i = 0; i < info->function_count; i++) {
        symbol_function_t *fn = &info->functions[i];
        uint16_t range;

        if (address < fn->start_address || address > fn->end_address)
            continue;

        range = fn->end_address - fn->start_address;
        if (fn->end_address == 0xFFFF)
            range = 0xFFFE;  /* Penalize sentinel, but still consider */

        if (!best || range < best_range) {
            best = fn;
            best_range = range;
        }
    }
    return best;
}

symbol_variable_t *
symbols_get_variables(symbol_function_t *func, int *count)
{
    if (!func) {
        if (count) *count = 0;
        return NULL;
    }
    if (count)
        *count = func->variable_count;
    return func->variables;
}
