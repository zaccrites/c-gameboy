
#ifndef CPU_INSTRUCTIONS
#define CPU_INSTRUCTIONS

struct Cpu;
typedef void (*InstructionImplFunc)(struct Cpu*);


struct Instruction
{
    const char *name;
    int numImmediateBytes;
    InstructionImplFunc impl;
};

const struct Instruction instructions[256];
const struct Instruction cbInstructions[256];


#endif
