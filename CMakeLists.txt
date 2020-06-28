
cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

project(c-gameboy VERSION 0.1.0 LANGUAGES C)


find_package(SDL2 REQUIRED)

add_executable(emulator
    src/main.c
    src/input.c
    src/graphics.c
)

target_compile_options(emulator PRIVATE
    -std=c11
    -pedantic
    -Wall
    -Wextra
    # -Werror
    -Werror=return-type
    -Werror=implicit-function-declaration
    -O2
)

target_link_libraries(emulator
    ${SDL2_LIBRARIES}
)
