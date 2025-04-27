#include <stdio.h>
#include <stdint.h>

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

    // Read and print first 32 bytes
    uint8_t buffer[32];
    size_t bytes_read = fread(buffer, 1, sizeof(buffer), file);
    fclose(file);

    printf("First %zu bytes of %s:\n", bytes_read, argv[1]);
    for (size_t i = 0; i < bytes_read; i++) {
        printf("%02x ", buffer[i]);
        if ((i + 1) % 16 == 0) printf("\n");
    }
    printf("\n");

    // Print magic number
    uint16_t magic = (uint16_t)buffer[0] | ((uint16_t)buffer[1] << 8);
    printf("Magic number: 0x%04x\n", magic);

    return 0;
} 