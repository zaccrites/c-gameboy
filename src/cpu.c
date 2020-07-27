
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "cpu.h"
#include "cpu_instructions.h"
#include "memory.h"


static uint8_t read_flags_byte(struct Cpu *cpu)
{
    uint8_t value = 0;
    if (cpu->flags.zero) { value |= (1 << 7); }
    if (cpu->flags.negative) { value |= (1 << 6); }
    if (cpu->flags.halfCarry) { value |= (1 << 5); }
    if (cpu->flags.carry) { value |= (1 << 4); }
    return value;
}

static void write_flags_byte(struct Cpu *cpu, uint8_t value)
{
    cpu->flags.zero = (value & (1 << 7)) != 0;
    cpu->flags.negative = (value & (1 << 6)) != 0;
    cpu->flags.halfCarry = (value & (1 << 5)) != 0;
    cpu->flags.carry = (value & (1 << 4)) != 0;
}


#define IO_REGISTER_INTERRUPT_FLAGS  0x0f
static uint8_t io_handler_read_interrupt_flags(IoRegisterFuncContext context)
{
    // TODO: mask?
    struct Cpu *cpu = context;
    return cpu->interruptFlags;
}
static void io_handler_write_interrupt_flags(IoRegisterFuncContext context, uint8_t value)
{
    // TODO: mask?
    struct Cpu *cpu = context;
    cpu->interruptFlags = value;
}


// Treat this like an IO register, even if it's technically not in the
// IO address space. It behaves like one (and if interrupt flags are
// an IO register, why not interrupt enable?).
#define IO_REGISTER_INTERRUPT_ENABLE  0xff
static uint8_t io_handler_read_interrupt_enable(IoRegisterFuncContext context)
{
    // TODO: mask?
    struct Cpu *cpu = context;
    return cpu->interruptEnable;
}
static void io_handler_write_interrupt_enable(IoRegisterFuncContext context, uint8_t value)
{
    // TODO: mask?
    struct Cpu *cpu = context;
    cpu->interruptEnable = value;
}


void cpu_init(struct Cpu *cpu, struct Memory *memory)
{
    cpu->pc = 0x0100;
    cpu->registers.a = 0x01;
    write_flags_byte(cpu, 0xb0);
    cpu->registers.b = 0x00;
    cpu->registers.c = 0x13;
    cpu->registers.d = 0x00;
    cpu->registers.e = 0xd8;
    cpu->registers.h = 0x01;
    cpu->registers.l = 0x4d;
    cpu->sp = 0xfffe;
    cpu->ime = true;
    cpu->halted = false;

    cpu->interruptFlags = 0x00;  // TODO
    cpu->interruptEnable = 0x00;  // TODO

    cpu->memory = memory;
    memory_register_io_handler(
        cpu->memory,
        IO_REGISTER_INTERRUPT_FLAGS,
        io_handler_read_interrupt_flags,
        io_handler_write_interrupt_flags,
        cpu
    );
    memory_register_io_handler(
        cpu->memory,
        IO_REGISTER_INTERRUPT_ENABLE,
        io_handler_read_interrupt_enable,
        io_handler_write_interrupt_enable,
        cpu
    );
}


uint16_t cpu_read_double_reg(struct Cpu *cpu, enum CpuDoubleRegister reg)
{
    uint16_t lowByte;
    uint16_t highByte;
    switch (reg)
    {
    case CPU_DOUBLE_REG_AF:
        highByte = cpu->registers.a;
        lowByte = read_flags_byte(cpu);
        break;
    case CPU_DOUBLE_REG_BC:
        highByte = cpu->registers.b;
        lowByte = cpu->registers.c;
        break;
    case CPU_DOUBLE_REG_DE:
        highByte = cpu->registers.d;
        lowByte = cpu->registers.e;
        break;
    case CPU_DOUBLE_REG_HL:
        highByte = cpu->registers.h;
        lowByte = cpu->registers.l;
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
    uint8_t highByte = value >> 8;
    switch (reg)
    {
    case CPU_DOUBLE_REG_AF:
        cpu->registers.a = highByte;
        write_flags_byte(cpu, lowByte);
        break;
    case CPU_DOUBLE_REG_BC:
        cpu->registers.b = highByte;
        cpu->registers.c = lowByte;
        break;
    case CPU_DOUBLE_REG_DE:
        cpu->registers.d = highByte;
        cpu->registers.e = lowByte;
        break;
    case CPU_DOUBLE_REG_HL:
        cpu->registers.h = highByte;
        cpu->registers.l = lowByte;
        break;
    default:
        assert(false);
        break;
    }
}


void cpu_push_dword(struct Cpu *cpu, uint16_t value)
{
    cpu->sp -= 2;
    memory_write_dword(cpu->memory, cpu->sp, value);
}

uint16_t cpu_pop_dword(struct Cpu *cpu)
{
    uint16_t value = memory_read_dword(cpu->memory, cpu->sp);
    cpu->sp += 2;
    return value;
}


void write_instruction_name_text(struct Cpu* cpu, const struct Instruction *instruction, char* buffer, size_t buflen)
{
    switch (instruction->numImmediateBytes)
    {
    case 0:
        strncpy(buffer, instruction->name, buflen - 1);
        break;
    case 1:
        snprintf(buffer, buflen, instruction->name, memory_read_word(cpu->memory, cpu->pc + 1));
        break;
    case 2:
        snprintf(buffer, buflen, instruction->name, memory_read_dword(cpu->memory, cpu->pc + 1));
        break;
    default:
        assert(false);
        break;
    }
}

void write_instruction_bytes_text(struct Cpu *cpu, const struct Instruction *instruction, char *buffer, size_t buflen)
{
    // TODO: Find a nicer solution
    uint16_t byteCount = 1 + instruction->numImmediateBytes;
    if (memory_read_word(cpu->memory, cpu->pc) == 0xcb)
    {
        byteCount += 1;
    }

    size_t cursor = 0;
    for (uint16_t i = 0; i < byteCount; i++)
    {
        uint8_t byte = memory_read_word(cpu->memory, cpu->pc + i);
        cursor += snprintf(buffer + cursor, buflen - cursor - 1, "%02x ", byte);
    }
    // Overwrite the last ' ' character.
    buffer[cursor - 1] = '\0';
}


void cpu_request_interrupt(struct Cpu *cpu, enum Interrupt interrupt)
{
    cpu->interruptFlags |= (1 << (uint8_t)interrupt);
}





static bool handle_interrupt(struct Cpu *cpu, enum Interrupt interrupt)
{
    cpu->interruptFlags &= ~(1 << (uint8_t)interrupt);
    cpu->ime = false;
    cpu_push_dword(cpu, cpu->pc);
    cpu->pc = 0x0040 + 0x0008 * (uint16_t)interrupt;;
    return true;
}


bool cpu_execute_next(struct Cpu *cpu)
{
    // TODO: Clean this up

    if (cpu->ime)
    {
        const enum Interrupt interrupts[] =
        {
            INTERRUPT_VBLANK,
            INTERRUPT_LCDC,
            INTERRUPT_TIMER,
            INTERRUPT_SERIAL,
            INTERRUPT_KEYPAD,
        };
        for (size_t i = 0; i < sizeof(interrupts) / sizeof(interrupts[0]); i++)
        {
            enum Interrupt interrupt = interrupts[i];
            bool interruptRequested = cpu->interruptFlags & (1 << (uint8_t)interrupt);
            bool interruptEnabled = cpu->interruptEnable & (1 << (uint8_t)interrupt);
            if (interruptRequested && interruptEnabled)
            {
                cpu->halted = false;
                return handle_interrupt(cpu, interrupt);
            }
        }
    }

    if (cpu->halted)
    {
        return true;
    }


    uint16_t pcStart = cpu->pc;

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
    bool isUnimplementedInstruction = instruction->impl == NULL;

    char instrNameBuffer[32];
    write_instruction_name_text(cpu, instruction, instrNameBuffer, sizeof(instrNameBuffer));

    char instrBytesBuffer[16];
    write_instruction_bytes_text(cpu, instruction, instrBytesBuffer, sizeof(instrBytesBuffer));

    // TOOD: Better way-- has to be after write_instruction_bytes_text, but before instruction->impl()
    cpu->pc += (opcode == 0xcb) ? 2 : 1;

    static bool tracing = false;
    // tracing = true;
    if (tracing || isUnimplementedInstruction)
    {
        // TODO: Move this outside the CPU and have it also print stuff related to
        // the rest of the system (P1 register, LY, LCDC, STAT, IE, IF, etc.)
        fprintf(
            stdout,
            "PC=%04x | %-8s | %-16s | AF=%04x (%c%c%c%c)  BC=%04x  DE=%04x  HL=%04x  SP=%04x  IME=%d \n",
            pcStart,
            instrBytesBuffer,
            instrNameBuffer,
            cpu_read_double_reg(cpu, CPU_DOUBLE_REG_AF),
            cpu->flags.zero ? 'Z' : 'z',
            cpu->flags.negative ? 'N': 'n',
            cpu->flags.halfCarry ? 'H' : 'h',
            cpu->flags.carry ? 'C' : 'c',
            cpu_read_double_reg(cpu, CPU_DOUBLE_REG_BC),
            cpu_read_double_reg(cpu, CPU_DOUBLE_REG_DE),
            cpu_read_double_reg(cpu, CPU_DOUBLE_REG_HL),
            cpu->sp,
            cpu->ime ? 1 : 0
        );
    }

    if (isUnimplementedInstruction)
    {
        fprintf(stderr, "Unimplemented instruction ([%s] = \"%s\")! Stopping. \n", instrBytesBuffer, instrNameBuffer);
        return false;
    }

    instruction->impl(cpu);
    return true;
}
