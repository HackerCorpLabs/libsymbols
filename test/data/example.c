#include <stdio.h>

// Global variables
int global_var = 42;
static int static_var = 24;

// Function declarations
int add_numbers(int a, int b);
void print_message(const char* msg);

// Main function
int main() {
    int local_var = 10;
    int result = add_numbers(local_var, global_var);
    print_message("Hello, World!");
    return result;
}

// Function definitions
int add_numbers(int a, int b) {
    return a + b;
}

void print_message(const char* msg) {
    printf("%s\n", msg);
} 