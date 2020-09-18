#!/bin/bash

# Run each of the individual test ROMs using the Python script.

# The bash script should fail if any of the individual tests fail.

# TODO: get current directory?
EMULATOR_PATH="/home/zac/c-gameboy-build/emulator"
ROMS_DIR="/home/zac/Code/c-gameboy/roms/blargg_test_roms/cpu_instrs/individual"

# TODO: Shore ROM names in an array and use loop.
# TODO: Add nicer output

# TODO: For some reason "E8 F8" replaces the eights with empty spaces in the output.
# Can I detect that in the Python script and fix it?
# What happens with other opcodes? Does a "9" show up as 0x21 instead of 0x20?


# python3 cpu_instructions.py "$EMULATOR_PATH" "$ROMS_DIR/01-special.gb"
python3 cpu_instructions.py "$EMULATOR_PATH" "$ROMS_DIR/02-interrupts.gb"
python3 cpu_instructions.py "$EMULATOR_PATH" "$ROMS_DIR/03-op sp,hl.gb"
# python3 cpu_instructions.py "$EMULATOR_PATH" "$ROMS_DIR/04-op r,imm.gb"
# python3 cpu_instructions.py "$EMULATOR_PATH" "$ROMS_DIR/05-op rp.gb"
# python3 cpu_instructions.py "$EMULATOR_PATH" "$ROMS_DIR/06-ld r,r.gb"
# python3 cpu_instructions.py "$EMULATOR_PATH" "$ROMS_DIR/07-jr,jp,call,ret,rst.gb"
# python3 cpu_instructions.py "$EMULATOR_PATH" "$ROMS_DIR/08-misc instrs.gb"
# python3 cpu_instructions.py "$EMULATOR_PATH" "$ROMS_DIR/09-op r,r.gb"
# python3 cpu_instructions.py "$EMULATOR_PATH" "$ROMS_DIR/10-bit ops.gb"
# python3 cpu_instructions.py "$EMULATOR_PATH" "$ROMS_DIR/11-op a,(hl).gb"
