
# c-gameboy

A Nintendo GameBoy emulator implemented in C


## Development

    apt install git build-essential cmake libsdl2-dev

    git clone git@github.com:zaccrites/c-gameboy.git
    cmake -S c-gameboy -B ~/c-gameboy-build
    make -C ~/c-gameboy-build


## Running

    ~/c-gameboy-build/emulator $PATH_TO_ROM_FILE
