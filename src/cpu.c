
#include <stdio.h>
#include <assert.h>

#include "cpu.h"
#include "cpu_instructions.h"


void cpu_init(struct Cpu *cpu, struct Memory *memory)
{
    cpu->pc = 0x0000;
    cpu->registers.a = 0xcc; // or whatever
    // ...
    cpu->memory = memory;
}


uint8_t cpu_read_reg(struct Cpu *cpu, enum CpuRegister reg)
{
    switch (reg)
    {
    case CPU_REG_A:
        return cpu->registers.a;
    case CPU_REG_B:
        return cpu->registers.b;
    case CPU_REG_C:
        return cpu->registers.c;
    case CPU_REG_D:
        return cpu->registers.d;
    case CPU_REG_E:
        return cpu->registers.e;
    case CPU_REG_H:
        return cpu->registers.h;
    case CPU_REG_L:
        return cpu->registers.l;
    default:
        assert(false);
        return 0;
    }
}


void cpu_write_reg(struct Cpu *cpu, enum CpuRegister reg, uint8_t value)
{
    switch (reg)
    {
    case CPU_REG_A:
        cpu->registers.a = value;
        break;
    case CPU_REG_B:
        cpu->registers.b = value;
        break;
    case CPU_REG_C:
        cpu->registers.c = value;
        break;
    case CPU_REG_D:
        cpu->registers.d = value;
        break;
    case CPU_REG_E:
        cpu->registers.e = value;
        break;
    case CPU_REG_H:
        cpu->registers.h = value;
        break;
    case CPU_REG_L:
        cpu->registers.l = value;
        break;
    default:
        assert(false);
        break;
    }
}



static uint8_t read_flags_byte(struct Cpu *cpu)
{
    // TODO
    (void)cpu;
    return 0;
}

static void write_flags_byte(struct Cpu *cpu, uint8_t value)
{
    // TODO
    (void)cpu;
    (void)value;
}



uint16_t cpu_read_double_reg(struct Cpu *cpu, enum CpuDoubleRegister reg)
{
    uint16_t lowByte;
    uint16_t highByte;
    switch (reg)
    {
    case CPU_DOUBLE_REG_AF:
        lowByte = cpu_read_reg(cpu, CPU_REG_A);
        highByte = read_flags_byte(cpu);
        break;
    case CPU_DOUBLE_REG_BC:
        lowByte = cpu_read_reg(cpu, CPU_REG_B);
        highByte = cpu_read_reg(cpu, CPU_REG_C);
        break;
    case CPU_DOUBLE_REG_DE:
        lowByte = cpu_read_reg(cpu, CPU_REG_D);
        highByte = cpu_read_reg(cpu, CPU_REG_E);
        break;
    case CPU_DOUBLE_REG_HL:
        lowByte = cpu_read_reg(cpu, CPU_REG_H);
        highByte = cpu_read_reg(cpu, CPU_REG_L);
        break;
    default:
        assert(false);
        return 0;
    }
    return (highByte << 8) | lowByte;
}


void cpu_write_double_reg(struct Cpu *cpu, enum CpuDoubleRegister reg, uint16_t value)
{
    uint8_t lowByte = value & 0xff;
    uint8_t highByte = value << 8;
    switch (reg)
    {
    case CPU_DOUBLE_REG_AF:
        cpu_write_reg(cpu, CPU_REG_A, lowByte);
        write_flags_byte(cpu, highByte);
        break;
    case CPU_DOUBLE_REG_BC:
        cpu_write_reg(cpu, CPU_REG_B, lowByte);
        cpu_write_reg(cpu, CPU_REG_C, highByte);
        break;
    case CPU_DOUBLE_REG_DE:
        cpu_write_reg(cpu, CPU_REG_D, lowByte);
        cpu_write_reg(cpu, CPU_REG_E, highByte);
        break;
    case CPU_DOUBLE_REG_HL:
        cpu_write_reg(cpu, CPU_REG_H, lowByte);
        cpu_write_reg(cpu, CPU_REG_L, highByte);
        break;
    default:
        assert(false);
        break;
    }
}




void cpu_execute_next(struct Cpu *cpu)
{
    const struct Instruction *instruction;
    uint8_t opcode = memory_read_word(cpu->memory, cpu->pc);
    if (opcode == 0xcb)
    {
        uint8_t cbOpcode = memory_read_word(cpu->memory, cpu->pc + 1);
        instruction = &cbInstructions[cbOpcode];
        assert(instruction->numImmediateBytes == 0);
    }
    else
    {
        instruction = &instructions[opcode];
    }

    // TODO: Clean up for general instruction trace printing
    if (instruction->impl == NULL)
    {
        char buffer[32];
        switch (instruction->numImmediateBytes)
        {
        case 0:
            strncpy(buffer, instruction->name, sizeof(buffer) - 1);
            break;
        case 1:
            snprintf(buffer, sizeof(buffer), instruction->name, memory_read_word(cpu->memory, cpu->pc + 1));
            break;
        case 2:
            snprintf(buffer, sizeof(buffer), instruction->name, memory_read_dword(cpu->memory, cpu->pc + 1));
            break;
        default:
            assert(false);
            break;
        }
        fprintf(stderr, "Unimplemented instruction: %s", buffer);
    }

    // TODO
    cpu->pc = cpu->pc + 1 + instruction->numImmediateBytes;
}
