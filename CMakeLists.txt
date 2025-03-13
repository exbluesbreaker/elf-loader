cmake_minimum_required(VERSION 3.10)

# Set the project name
project(elf_loader)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_compile_options(-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Werror)

# Add the executable
add_executable(elf_loader main.cpp ELFParser.cpp)

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Link libraries (if any)
# target_link_libraries(elf-loader <library_name>)