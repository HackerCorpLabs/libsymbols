cmake_minimum_required(VERSION 3.14)
project(symbols VERSION 0.1.0 LANGUAGES C)

# Set C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Platform-specific settings
if(WIN32)
    # Windows-specific settings
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
else()
    # Linux/Unix specific settings
    add_compile_definitions(_GNU_SOURCE)
endif()

# Define the source files
file(GLOB SYMBOLS_SOURCES "src/*.c")
set(SYMBOLS_HEADERS "include/symbols.h" "include/mapfile.h" "include/nlist.h" "include/stabs.h")

# Create an object library
add_library(symbols_objects OBJECT ${SYMBOLS_SOURCES})

# Set include directories
target_include_directories(symbols_objects
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Add compiler options
target_compile_options(symbols_objects PRIVATE
    $<$<CONFIG:Debug>:-g -O0>
    $<$<CONFIG:Release>:-O2>
)

# Add warning flags depending on compiler
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(symbols_objects PRIVATE -Wall -Wextra -pedantic)
elseif(MSVC)
    target_compile_options(symbols_objects PRIVATE /W4)
endif()

# Build test executable if requested
option(BUILD_SYMBOLS_TEST "Build symbol library test code" OFF)

if(BUILD_SYMBOLS_TEST)
    message(STATUS "Building symbols test tools")
    
    # Compile test code
    file(GLOB TEST_SOURCES "test/*.c")
    
    # Create test executables
    foreach(test_src ${TEST_SOURCES})
        get_filename_component(test_name ${test_src} NAME_WE)
        add_executable(${test_name} ${test_src})
        target_link_libraries(${test_name} PRIVATE symbols_objects)
        target_include_directories(${test_name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    endforeach()
endif()

# Install rules for headers
install(FILES ${SYMBOLS_HEADERS} 
        DESTINATION include/symbols)

# Install rules for the object library
install(TARGETS symbols_objects
        EXPORT symbols_objects-export
        OBJECTS DESTINATION lib
        PUBLIC_HEADER DESTINATION include/symbols) 