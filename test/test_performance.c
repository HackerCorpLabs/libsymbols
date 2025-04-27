#include "../include/symbols.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

// Number of symbols to generate
#define NUM_SYMBOLS 10000
// Number of lookups to perform
#define NUM_LOOKUPS 1000

// Helper function to generate random addresses
static uint16_t random_address(void) {
    return (uint16_t)(rand() % 0xFFFF);
}

// Helper function to measure lookup time
static double measure_lookup_time(const symbol_table_t* table, uint16_t address) {
    clock_t start = clock();
    symbols_lookup_by_address(table, address);  // We don't need the result for timing
    clock_t end = clock();
    return (double)(end - start) / CLOCKS_PER_SEC;
}

int main(void) {
    // Initialize random number generator
    srand(time(NULL));

    // Create symbol table
    symbol_table_t* table = symbols_create();
    assert(table != NULL);

    // Generate random symbols
    printf("Generating %d random symbols...\n", NUM_SYMBOLS);
    for (int i = 0; i < NUM_SYMBOLS; i++) {
        char name[32];
        char filename[32];
        snprintf(name, sizeof(name), "func_%d", i);
        snprintf(filename, sizeof(filename), "test_%d.c", i);
        
        assert(symbols_add_entry(table, filename, name, i, random_address(), SYMBOL_TYPE_FUNCTION));
    }
    printf("Done.\n\n");

    // Generate random addresses to look up
    uint16_t* addresses = malloc(NUM_LOOKUPS * sizeof(uint16_t));
    assert(addresses != NULL);
    for (int i = 0; i < NUM_LOOKUPS; i++) {
        addresses[i] = random_address();
    }

    // Test linear search
    printf("Testing linear search...\n");
    double linear_total = 0.0;
    for (int i = 0; i < NUM_LOOKUPS; i++) {
        linear_total += measure_lookup_time(table, addresses[i]);
    }
    printf("Linear search average time: %.6f seconds\n\n", 
           linear_total / NUM_LOOKUPS);

    // Sort entries for binary search
    printf("Sorting entries for binary search...\n");
    qsort(table->entries, table->count, sizeof(symbol_entry_t), compare_entries_by_address);

    // Test binary search
    printf("Testing binary search...\n");
    double binary_total = 0.0;
    for (int i = 0; i < NUM_LOOKUPS; i++) {
        binary_total += measure_lookup_time(table, addresses[i]);
    }
    printf("Binary search average time: %.6f seconds\n\n", 
           binary_total / NUM_LOOKUPS);

    // Calculate speedup
    double speedup = linear_total / binary_total;
    printf("Binary search is %.2fx faster than linear search\n", speedup);

    // Cleanup
    free(addresses);
    symbols_free(table);

    return 0;
} 