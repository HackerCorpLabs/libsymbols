#include <stdio.h>
#include <stdint.h>
#include <ctype.h>
#include "../include/aout.h"

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        return 1;
    }

    FILE* file = fopen(argv[1], "rb");
    if (!file) {
        perror("Failed to open file");
        return 1;
    }

    // Read and print a.out header
    aout_header_t header;
    if (load_header(file, &header, 1) != 0) {
        fprintf(stderr, "Failed to read a.out header\n");
        fclose(file);
        return 1;
    }

    // Read and print first 256 bytes (for a longer dump)
    fseek(file, 0, SEEK_SET); // rewind to start for raw dump
    uint8_t buffer[256];
    size_t bytes_read = fread(buffer, 1, sizeof(buffer), file);
    fclose(file);

    printf("Offset(o) ");
    for (int i = 0; i < 16; i++) printf(" %03o", i);
    printf("  | Decoded text\n");

    for (size_t offset = 0; offset < bytes_read; offset += 16) {
        // Print offset in octal
        printf("%08zo  ", offset);
        // Print octal values
        for (size_t i = 0; i < 16; i++) {
            if (offset + i < bytes_read)
                printf(" %03o", buffer[offset + i]);
            else
                printf("    ");
        }
        printf("  | ");
        // Print ASCII representation with byte swap in pairs
        for (size_t i = 0; i < 16; i += 2) {
            if (offset + i + 1 < bytes_read) {
                uint8_t b = buffer[offset + i + 1];
                uint8_t a = buffer[offset + i];
                printf("%c%c", isprint(b) ? b : '.', isprint(a) ? a : '.');
            } else if (offset + i < bytes_read) {
                uint8_t a = buffer[offset + i];
                printf("%c ", isprint(a) ? a : '.');
            } else {
                printf("  ");
            }
        }
        printf("\n");
    }

    // Print magic number
    if (bytes_read >= 2) {
        uint16_t magic = (uint16_t)buffer[0] | ((uint16_t)buffer[1] << 8);
        printf("\nMagic number: 0x%04x\n", magic);
    }

    return 0;
} 