CC = gcc
CFLAGS = -Wall -Wextra -Iinclude
AR = ar
ARFLAGS = rcs

SRC_DIR = src
OBJ_DIR = obj
LIB_DIR = .
BUILD_DIR = build

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))
LIB = $(LIB_DIR)/libsymbols.a

# CMake options
CMAKE = cmake
CMAKE_BUILD_TYPE = Release

.PHONY: all clean test cmake cmake-debug cmake-test cmake-clean help

# Default target uses traditional Makefile build
all: $(LIB)

$(LIB): $(OBJS)
	$(AR) $(ARFLAGS) $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# CMake targets
cmake:
	@mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR) && $(CMAKE) -DCMAKE_BUILD_TYPE=$(CMAKE_BUILD_TYPE) ..
	cd $(BUILD_DIR) && $(CMAKE) --build .

cmake-debug:
	@mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR) && $(CMAKE) -DCMAKE_BUILD_TYPE=Debug ..
	cd $(BUILD_DIR) && $(CMAKE) --build .

cmake-test:
	@mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR) && $(CMAKE) -DCMAKE_BUILD_TYPE=$(CMAKE_BUILD_TYPE) -DBUILD_SYMBOLS_TEST=ON ..
	cd $(BUILD_DIR) && $(CMAKE) --build .

cmake-clean:
	rm -rf $(BUILD_DIR)

# Traditional clean and test targets
clean: cmake-clean
	rm -rf $(OBJ_DIR) $(LIB)
	$(MAKE) -C test clean

test: $(LIB)
	$(MAKE) -C test

# Help message
help:
	@echo "Available targets:"
	@echo "  all          - Build the library using traditional make (default)"
	@echo "  test         - Build and run tests using traditional make"
	@echo "  clean        - Clean all build artifacts"
	@echo "  cmake        - Build using CMake with Release configuration"
	@echo "  cmake-debug  - Build using CMake with Debug configuration"
	@echo "  cmake-test   - Build the library and tests using CMake"
	@echo "  cmake-clean  - Clean CMake build directory"
	@echo "  help         - Show this help message" 