#!/bin/bash

# Compile with STABS debug info and generate assembly
gcc -gstabs -S example.c -o example.s

# Compile to a.out format
gcc -gstabs example.c -o example.out

# Create a simple map file
cat > example.map << EOF
# Simple map file example
# Format: address symbol_name
0x1000 main
0x1100 add_numbers
0x1200 print_message
0x1300 global_var
EOF

echo "Generated test files:"
echo "- example.s (STABS debug info)"
echo "- example.out (a.out binary)"
echo "- example.map (symbol map)" 