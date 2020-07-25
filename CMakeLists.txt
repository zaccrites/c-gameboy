
cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

project(c-gameboy VERSION 0.1.0 LANGUAGES C)


find_package(SDL2 REQUIRED)

add_executable(emulator
    src/main.c
    src/input.c
    src/graphics.c
    src/cartridge.c
    src/memory.c
    src/cpu.c
    src/cpu_instructions.c
    src/ppu.c
    src/keypad.c
    src/dma.c
    src/timer.c
)

target_compile_options(emulator PRIVATE
    -std=c99
    -pedantic
    -O2
    -fstrict-aliasing

    -Wall
    -Wextra
    -Werror

    -Wbad-function-cast
    -Wcast-align
    -Wcast-qual
    -Wfloat-equal
    -Winline
    -Wlogical-op
    -Wnested-externs
    -Wold-style-declaration
    -Wold-style-definition
    -Wpointer-arith
    -Wredundant-decls
    -Wshadow
    -Wstrict-aliasing=2
    -Wstrict-overflow=5
    -Wstrict-prototypes
    -Wswitch-default
    -Wswitch-enum
    -Wunreachable-code
)

target_link_libraries(emulator
    ${SDL2_LIBRARIES}
)
