# libsymbols

A C library for parsing and managing debug symbol information from various formats.

## Features

- Parse STABS debug information from .s files
- Read symbol tables from a.out binaries
- Load binary code from a.out files
- Load custom map files
- Fast binary search for symbol lookups
- Support for DAP (Debug Adapter Protocol) integration

## DAP Integration

### Launch Phase

The library supports loading debug symbols during the DAP launch phase:

```c
// Load symbols from a.out binary
bool symbols_load_aout(const char* filename);

// Load symbols from STABS .s file
bool symbols_load_stabs(const char* filename);

// Load symbols from map file
bool symbols_load_map(const char* filename);
```

### Breakpoint Support

For setting breakpoints, the library provides:

```c
// Find address for a source location
uint16_t symbols_find_address(const symbol_table_t* table, 
                            const char* filename, int line);

// Get source location for an address
const char* symbols_get_file(const symbol_table_t* table, uint16_t address);
int symbols_get_line(const symbol_table_t* table, uint16_t address);
```

### Stepping Support

For stepping through code:

```c
// Look up symbol at address
const symbol_entry_t* symbols_lookup_by_address(const symbol_table_t* table, 
                                              uint16_t address);

// Check if address is a line entry
bool symbols_is_line_entry(const symbol_entry_t* entry);

// Get next line address for stepping
uint16_t symbols_get_next_line_address(const symbol_table_t* table, 
                                     uint16_t current_address);
```

## Binary Loading

The library can load binary code from a.out files:

```c
// Load binary code
binary_info_t info;
if (symbols_load_binary("program.out", &info)) {
    // Get program entry point
    uint16_t entry = symbols_get_entry_point(&info);
    
    // Find memory segment for an address
    const memory_segment_t* segment = symbols_get_segment(&info, 0x1000);
    if (segment) {
        // Access memory contents
        uint8_t value = segment->data[0x1000 - segment->start_address];
    }
    
    // Clean up
    symbols_free_binary(&info);
}
```

The binary loading features provide:
- Loading of text and data segments
- Memory segment information (start address, size, type)
- Program entry point
- Memory access through segment data pointers

## Building

```bash
make
```

## Testing

The library comes with a comprehensive test suite. To run all tests:

1. First build the library and tests:
```bash
make
cd test
make
```

2. Run individual test programs:
```bash
# Test map file parser
./test_mapfile [mapfile]

# Test a.out symbol loading and binary code loading
./test_symbols_aout [a.out file] [--dump-code]

# Run performance tests
./test_performance
```

Each test program accepts different command line arguments:

- `test_mapfile`: Tests the map file parser. If no file is specified, it uses a default test file.
- `test_symbols_aout`: Tests a.out symbol loading and binary code loading. The `--dump-code` option dumps the loaded binary code.
- `test_performance`: Runs performance tests for symbol lookups and other operations.

Test data files are located in the `test/data/` directory.

## DAP Server Integration Example

```c
// Launch phase
symbol_table_t* symbols = symbols_create();
if (symbols_load_aout(program_path)) {
    // Symbols loaded successfully
} else if (symbols_load_stabs(stabs_path)) {
    // STABS symbols loaded
} else if (symbols_load_map(map_path)) {
    // Map file loaded
}

// Breakpoint handling
uint16_t address = symbols_find_address(symbols, source_file, line_number);
if (address != 0) {
    // Set breakpoint at address
}

// Stepping
uint16_t pc = get_current_pc();
const symbol_entry_t* entry = symbols_lookup_by_address(symbols, pc);
if (entry) {
    // Update source location
    current_file = entry->filename;
    current_line = entry->line;
}
```

## Symbol Types

The library supports various symbol types:

- Functions (N_FUN)
- Variables (N_GSYM, N_LSYM)
- Source files (N_SO)
- Line numbers (N_SLINE)
- Types (N_TYPE)

## Memory Management

The library handles memory management for:
- Symbol tables
- String allocations
- Type information
- Source file paths

## Performance

The library uses binary search for fast symbol lookups, providing:
- O(log n) lookup time for address-based searches
- Automatic sorting of symbols by address
- Efficient memory usage
- Linear search for name-based lookups (since names are not sorted)

## License

MIT License

## Overview

This library provides a unified interface for reading and manipulating debug symbols from different formats:
- STABS (.s files)
- a.out symbol tables (nlist)
- Map files (source:line -> address mappings)

## Current Status

### Completed Features
- ✅ Basic symbol table structure and management
- ✅ STABS parser implementation for PCC compiler output
- ✅ a.out nlist parser implementation
- ✅ Map file parser implementation
- ✅ Unified symbol type system
- ✅ Basic symbol lookup functions
- ✅ Memory management and cleanup
- ✅ Binary search optimization for address lookups
- ✅ Binary code loading from a.out files
- ✅ Test suite for all major components
- ✅ Integration with ND-100 toolchain

### In Progress
- 🔄 Documentation improvements
- 🔄 Binary loading bug fixes (see Todo section)
- 🔄 Performance optimization for large symbol tables

### Todo
- Fix logic to load binary
  - test_symbols_aout --dump-code data/intr.out
  - Debug: text=171 data=0 bss=0 syms=184
  - Failed to load binary from data/intr.out
- Add support for more complex STABS expressions
- Implement caching for frequently accessed symbols
- Add support for DWARF debug information (optional)

## Building

The library can be built using the provided Makefile:

```bash
make
```

This will build both the library and test programs.

## Usage

Basic usage example:

```c
#include "symbols.h"

int main() {
    // Create a new symbol table
    symbol_table_t* table = symbols_create();
    if (!table) {
        // Handle error
    }

    // Load symbols from different sources
    if (!symbols_load_stabs(table, "debug.s")) {
        // Handle error
    }

    if (!symbols_load_nlist(table, "program.out")) {
        // Handle error
    }

    if (!symbols_load_map(table, "program.map")) {
        // Handle error
    }

    // Look up symbols
    const symbol_entry_t* symbol = symbols_lookup_by_address(table, 0x1234);
    if (symbol) {
        printf("Found symbol at 0x1234: %s\n", symbol->name);
    }

    // Clean up
    symbols_free(table);
    return 0;
}
```

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## License

This project is licensed under the MIT License - see the LICENSE file for details.

---

## ✅ Goals

- Parse STABS debug symbols from `.s` files (produced by PCC compiler)
- Parse `nlist` entries from a.out binaries
- Spport simple map file formats
- Provide a unified lookup API: address ↔ file/line, symbol name ↔ address
- Be usable by:
  - ND-100 emulator
  - DAP server (`libdap`)
  - Symbol inspection tools (e.g., `test_reader`)

---

## 📁 Directory Structure

```
libsymbols/
├── include/         # Public headers (symbols.h)
├── src/             # Implementation files (stabs, nlist, map, core logic)
├── test/            # Test program to load and dump symbols
├── Makefile         # Builds libsymbols.a
├── README.md        # This file
└── docs/            # Optional: references, notes, and format docs
```

---

## 📌 Step-by-Step Plan


---

### Phase 1: Implement Core Symbol System

7. [ ] Implement `SymbolEntry` data structure (shared memory model)
8. [ ] Implement symbol table loading for:
    - [ ] STABS in `.s` files
    - [ ] `nlist` in a.out binaries
    - [ ] Optional: address map files
9. [ ] Implement lookup functions:
    - [ ] `symbols_lookup_by_address()`
    - [ ] `symbols_lookup_by_name()`
    - [ ] `symbols_find_address(file, line)`
10. [ ] Implement `symbols_dump_all()` for inspection

---

### Phase 2: Integrate With ND-100 Toolchain

11. [ ] Export the library for use in:
    - ND-100 emulator
    - `libdap` DAP server
12. [ ] Add error handling and validation
13. [ ] Document the public API with comments
14. [ ] Add test samples to `test/` folder

---

## 📚 References

### STABS Format:
- [STABS documentation (GDB Manual)](https://sourceware.org/gdb/current/onlinedocs/stabs.html)
- [GCC Debugging Information](https://gcc.gnu.org/onlinedocs/gccint/Debugging-Information.html)
- [STAB constants in stab.h (NetBSD)](https://github.com/NetBSD/src/blob/trunk/include/stab.h)

### a.out & nlist:
- [nlist(3) man page](https://man7.org/linux/man-pages/man3/nlist.3.html)
- [nlist.h (NetBSD)](https://github.com/NetBSD/src/blob/trunk/include/nlist.h)
- [a.out format summary (OSDev)](https://wiki.osdev.org/A.out)

---

## 🛠 License

MIT License 

---

## 📦 Planned Future Features

- STABS expression support (for types and structs)
- DWARF support (optional, later)
- Type-safe access to variable memory in emulator
