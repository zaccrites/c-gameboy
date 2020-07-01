
#ifndef CPU_INSTRUCTIONS
#define CPU_INSTRUCTIONS

struct Cpu;
// typedef int (*InstructionImplFunc)(struct Cpu*);
typedef void (*InstructionImplFunc)(struct Cpu*);


struct Instruction
{
    const char *name;
    int numImmediateBytes;
    InstructionImplFunc impl;

    // Name
    // Size in bytes
    // Addressing mode
    // Target registers
    // Timing info
    // Function pointer to implementation


    // Alternatively, forget the tables and just create a single function
    // for each kind of instruction that handles its own timing and target
    // registers and all that stuff in a way that makes sense for that instruction.
};

const struct Instruction instructions[256];
const struct Instruction cbInstructions[256];


#endif
