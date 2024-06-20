cmake_minimum_required(VERSION 3.28.0)
project(voxel_game VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_PDB_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/pdb")

# Warnings
set(CMAKE_COMPILE_WARNING_AS_ERROR ON)

if (MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -pedantic)
endif()

# === Targets ===
# Modules
add_library(modules OBJECT)

find_package(glad CONFIG REQUIRED)
target_link_libraries(modules PUBLIC glad::glad)

find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(modules PUBLIC glfw)

find_package(glm CONFIG REQUIRED)
target_link_libraries(modules PUBLIC glm::glm)

add_subdirectory("modules/")

# Game executable
add_executable(voxel_game)

target_link_libraries(voxel_game PRIVATE modules)

add_subdirectory("src/")