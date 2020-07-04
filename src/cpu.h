
#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include <stdbool.h>

struct Memory;


enum Interrupt
{
    INTERRUPT_VBLANK = 0,
    INTERRUPT_LCDC   = 1,
    INTERRUPT_TIMER  = 2,
    INTERRUPT_SERIAL = 3,
    INTERRUPT_KEYPAD = 4,
};


enum CpuDoubleRegister
{
    CPU_DOUBLE_REG_AF,
    CPU_DOUBLE_REG_BC,
    CPU_DOUBLE_REG_DE,
    CPU_DOUBLE_REG_HL,
};


struct Cpu
{
    uint16_t pc;
    uint16_t sp;

    struct
    {
        uint8_t a;
        uint8_t b;
        uint8_t c;
        uint8_t d;
        uint8_t e;
        uint8_t h;
        uint8_t l;
    } registers;

    struct
    {
        bool zero;
        bool negative;
        bool halfCarry;
        bool carry;
    } flags;

    bool ime;

    uint8_t interruptFlags;
    uint8_t interruptEnable;

    struct Memory *memory;
};


void cpu_init(struct Cpu *cpu, struct Memory *memory);
bool cpu_execute_next(struct Cpu *cpu);
void cpu_request_interrupt(struct Cpu *cpu, enum Interrupt interrupt);

uint16_t cpu_read_double_reg(struct Cpu *cpu, enum CpuDoubleRegister reg);
void cpu_write_double_reg(struct Cpu *cpu, enum CpuDoubleRegister reg, uint16_t value);
void cpu_push_dword(struct Cpu *cpu, uint16_t value);
uint16_t cpu_pop_dword(struct Cpu *cpu);

#endif
