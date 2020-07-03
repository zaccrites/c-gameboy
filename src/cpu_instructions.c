
#include "cpu_instructions.h"
#include "cpu.h"


static uint8_t read_mem_at_hl(struct Cpu *cpu)
{
    uint16_t address = cpu_read_double_reg(cpu, CPU_DOUBLE_REG_HL);
    return memory_read_word(cpu->memory, address);
}

static void write_mem_at_hl(struct Cpu *cpu, uint8_t value)
{
    uint16_t address = cpu_read_double_reg(cpu, CPU_DOUBLE_REG_HL);
    memory_write_word(cpu->memory, address, value);
}


static uint8_t imm_word(struct Cpu *cpu)
{
    return memory_read_word(cpu->memory, cpu->pc++);
}

static uint16_t imm_dword(struct Cpu *cpu)
{
    uint16_t value = memory_read_dword(cpu->memory, cpu->pc);
    cpu->pc += 2;
    return value;
}


static uint16_t modify_double_reg(struct Cpu *cpu, enum CpuDoubleRegister reg, uint16_t change)
{
    uint16_t value = cpu_read_double_reg(cpu, reg) + change;
    cpu_write_double_reg(cpu, reg, value);
    return value;
}


// #define INSTR(name)  static void name(struct Cpu *cpu)


static void push_dword(struct Cpu *cpu, uint16_t value)
{
    memory_write_dword(cpu->memory, cpu->sp, value);
    cpu->sp -= 2;
}



static void nop(struct Cpu *cpu)
{
    // Do nothing
    (void)cpu;
}

static void di(struct Cpu *cpu)
{
    // TODO: Wait to disable interrupts until the next instruction completes
    cpu->ime = false;
}

static void ei(struct Cpu *cpu)
{
    // TODO: Wait to enable interrupts until the next instruction completes
    cpu->ime = true;
}


static void jp_a16(struct Cpu *cpu)
{
    cpu->pc = imm_dword(cpu);
}


static void _jr(struct Cpu *cpu, bool condition)
{
    int8_t offset = imm_word(cpu);
    if (condition)
    {
        cpu->pc += offset;
    }
}
static void jr(struct Cpu *cpu) { _jr(cpu, true); }
static void jr_z(struct Cpu *cpu) { _jr(cpu, cpu->flags.zero); }
static void jr_nz(struct Cpu *cpu) { _jr(cpu, ! cpu->flags.zero); }
static void jr_c(struct Cpu *cpu) { _jr(cpu, cpu->flags.carry); }
static void jr_nc(struct Cpu *cpu) { _jr(cpu, ! cpu->flags.carry); }


static void rst(struct Cpu* cpu, uint16_t address)
{
    push_dword(cpu, cpu->pc);
    cpu->pc = address;
}
static void rst00(struct Cpu *cpu) { rst(cpu, 0x0000); }
static void rst08(struct Cpu *cpu) { rst(cpu, 0x0008); }
static void rst10(struct Cpu *cpu) { rst(cpu, 0x0010); }
static void rst18(struct Cpu *cpu) { rst(cpu, 0x0018); }
static void rst20(struct Cpu *cpu) { rst(cpu, 0x0020); }
static void rst28(struct Cpu *cpu) { rst(cpu, 0x0028); }
static void rst30(struct Cpu *cpu) { rst(cpu, 0x0030); }
static void rst38(struct Cpu *cpu) { rst(cpu, 0x0038); }


static void xor(struct Cpu *cpu, uint8_t value)
{
    cpu->registers.a ^= value;
    cpu->flags.zero = cpu->registers.a == 0;
    cpu->flags.negative = false;
    cpu->flags.halfCarry = false;
    cpu->flags.carry = false;
}
static void xor_a(struct Cpu *cpu) { xor(cpu, cpu->registers.a); }
static void xor_b(struct Cpu *cpu) { xor(cpu, cpu->registers.b); }
static void xor_c(struct Cpu *cpu) { xor(cpu, cpu->registers.c); }
static void xor_d(struct Cpu *cpu) { xor(cpu, cpu->registers.d); }
static void xor_e(struct Cpu *cpu) { xor(cpu, cpu->registers.e); }
static void xor_h(struct Cpu *cpu) { xor(cpu, cpu->registers.h); }
static void xor_l(struct Cpu *cpu) { xor(cpu, cpu->registers.l); }
static void xor_hl(struct Cpu *cpu) { xor(cpu, read_mem_at_hl(cpu)); }


static void ld_hl_d16(struct Cpu *cpu) { cpu_write_double_reg(cpu, CPU_DOUBLE_REG_HL, imm_dword(cpu)); }

static void ld_a_d8(struct Cpu *cpu) { cpu->registers.a = imm_word(cpu); }
static void ld_b_d8(struct Cpu *cpu) { cpu->registers.b = imm_word(cpu); }
static void ld_c_d8(struct Cpu *cpu) { cpu->registers.c = imm_word(cpu); }
static void ld_d_d8(struct Cpu *cpu) { cpu->registers.d = imm_word(cpu); }
static void ld_e_d8(struct Cpu *cpu) { cpu->registers.e = imm_word(cpu); }
static void ld_h_d8(struct Cpu *cpu) { cpu->registers.h = imm_word(cpu); }
static void ld_l_d8(struct Cpu *cpu) { cpu->registers.l = imm_word(cpu); }
static void ld_hl_d8(struct Cpu *cpu) { write_mem_at_hl(cpu, imm_word(cpu)); }

static void ld_hlm_a(struct Cpu *cpu)
{
    write_mem_at_hl(cpu, cpu->registers.a);
    modify_double_reg(cpu, CPU_DOUBLE_REG_HL, -1);
}
static void ld_hlp_a(struct Cpu *cpu)
{
    write_mem_at_hl(cpu, cpu->registers.a);
    modify_double_reg(cpu, CPU_DOUBLE_REG_HL, +1);
}
static void ld_a_hlm(struct Cpu *cpu)
{
    cpu->registers.a = read_mem_at_hl(cpu);
    modify_double_reg(cpu, CPU_DOUBLE_REG_HL, -1);
}
static void ld_a_hlp(struct Cpu *cpu)
{
    cpu->registers.a = read_mem_at_hl(cpu);
    modify_double_reg(cpu, CPU_DOUBLE_REG_HL, +1);
}


static void ldh_a8_a(struct Cpu *cpu)
{
    uint16_t address = 0xff00 | imm_word(cpu);
    memory_write_word(cpu->memory, address, cpu->registers.a);
}

static void ldh_a_a8(struct Cpu *cpu)
{
    uint16_t address = 0xff00 | imm_word(cpu);
    cpu->registers.a = memory_read_word(cpu->memory, address);
}


static uint8_t dec(struct Cpu *cpu, uint8_t value)
{
    uint8_t newValue = value - 1;
    cpu->flags.zero = newValue == 0;
    cpu->flags.negative = true;
    cpu->flags.halfCarry = (value & 0x0f) == 0x00;
    // carry flag not affected
    return newValue;
}
static void dec_a(struct Cpu *cpu) { cpu->registers.a = dec(cpu, cpu->registers.a); }
static void dec_b(struct Cpu *cpu) { cpu->registers.b = dec(cpu, cpu->registers.b); }
static void dec_c(struct Cpu *cpu) { cpu->registers.c = dec(cpu, cpu->registers.c); }
static void dec_d(struct Cpu *cpu) { cpu->registers.d = dec(cpu, cpu->registers.d); }
static void dec_e(struct Cpu *cpu) { cpu->registers.e = dec(cpu, cpu->registers.e); }
static void dec_h(struct Cpu *cpu) { cpu->registers.h = dec(cpu, cpu->registers.h); }
static void dec_l(struct Cpu *cpu) { cpu->registers.l = dec(cpu, cpu->registers.l); }
static void dec_mem_hl(struct Cpu *cpu)
{
    uint8_t value = read_mem_at_hl(cpu);
    value = dec(cpu, value);
    write_mem_at_hl(cpu, value);
}


static uint8_t inc(struct Cpu *cpu, uint8_t value)
{
    uint8_t newValue = value + 1;
    cpu->flags.zero = newValue == 0;
    cpu->flags.negative = true;
    cpu->flags.halfCarry = (value & 0x0f) == 0x0f;
    // carry flag not affected
    return newValue;
}
static void inc_a(struct Cpu *cpu) { cpu->registers.a = inc(cpu, cpu->registers.a); }
static void inc_b(struct Cpu *cpu) { cpu->registers.b = inc(cpu, cpu->registers.b); }
static void inc_c(struct Cpu *cpu) { cpu->registers.c = inc(cpu, cpu->registers.c); }
static void inc_d(struct Cpu *cpu) { cpu->registers.d = inc(cpu, cpu->registers.d); }
static void inc_e(struct Cpu *cpu) { cpu->registers.e = inc(cpu, cpu->registers.e); }
static void inc_h(struct Cpu *cpu) { cpu->registers.h = inc(cpu, cpu->registers.h); }
static void inc_l(struct Cpu *cpu) { cpu->registers.l = inc(cpu, cpu->registers.l); }
static void inc_mem_hl(struct Cpu *cpu)
{
    uint8_t value = read_mem_at_hl(cpu);
    value = inc(cpu, value);
    write_mem_at_hl(cpu, value);
}


static void cp(struct Cpu *cpu, uint8_t value)
{
    cpu->flags.zero = cpu->registers.a == value;
    cpu->flags.negative = true;
    cpu->flags.halfCarry = (value & 0x0f) > (cpu->registers.a & 0x0f);
    cpu->flags.carry = cpu->registers.a < value;
}
static void cp_a(struct Cpu *cpu) { cp(cpu, cpu->registers.a); }
static void cp_b(struct Cpu *cpu) { cp(cpu, cpu->registers.b); }
static void cp_c(struct Cpu *cpu) { cp(cpu, cpu->registers.c); }
static void cp_d(struct Cpu *cpu) { cp(cpu, cpu->registers.d); }
static void cp_e(struct Cpu *cpu) { cp(cpu, cpu->registers.e); }
static void cp_h(struct Cpu *cpu) { cp(cpu, cpu->registers.h); }
static void cp_l(struct Cpu *cpu) { cp(cpu, cpu->registers.l); }
static void cp_mem_hl(struct Cpu *cpu) { cp(cpu, read_mem_at_hl(cpu)); }
static void cp_d8(struct Cpu* cpu) { cp(cpu, imm_word(cpu)); }


const struct Instruction instructions[256] = {
    // 0x00
    { "NOP",             0, nop },
    { "LD BC, 0x%04x",   2, NULL },
    { "LD (BC), A",      0, NULL },
    { "INC BC",          0, NULL },
    { "INC B",           0, inc_b },
    { "DEC B",           0, dec_b },
    { "LD B, 0x%02x",    1, ld_b_d8 },
    { "RLCA",            0, NULL },
    { "LD (0x%04x), SP", 2, NULL },
    { "ADD HL, BC",      0, NULL },
    { "LD A, (BC)",      0, NULL },
    { "DEC BC",          0, NULL },
    { "INC C",           0, inc_c },
    { "DEC C",           0, dec_c },
    { "LD C, 0x%02x",    1, ld_c_d8 },
    { "RRCA",            0, NULL },

    // 0x10
    { "STOP %d",         1, NULL },
    { "LD DE, 0x%04x",   2, NULL },
    { "LD (DE), A",      0, NULL },
    { "INC DE",          0, NULL },
    { "INC D",           0, inc_d },
    { "DEC D",           0, dec_d },
    { "LD D, 0x%02x",    1, ld_d_d8 },
    { "RLA",             0, NULL },
    { "JR %hhd",         1, jr },
    { "ADD HL, DE",      0, NULL },
    { "LD A, (DE)",      0, NULL },
    { "DEC DE",          0, NULL },
    { "INC E",           0, inc_e },
    { "DEC E",           0, dec_e },
    { "LD E, 0x%02x",    1, ld_e_d8 },
    { "RRA",             0, NULL },

    // 0x20
    { "JR NZ, %hhd",     1, jr_nz },
    { "LD HL, 0x%04x",   2, ld_hl_d16 },
    { "LD (HL+), A",     0, ld_hlp_a },
    { "INC HL",          0, NULL },
    { "INC H",           0, inc_h },
    { "DEC H",           0, dec_h },
    { "LD H, 0x%02x",    1, ld_h_d8 },
    { "DAA",             0, NULL },
    { "JR Z, %hhd",      1, jr_z },
    { "ADD HL, HL",      0, NULL },
    { "LD A, (HL+)",     0, ld_a_hlp },
    { "DEC HL",          0, NULL },
    { "INC L",           0, inc_l },
    { "DEC L",           0, dec_l },
    { "LD L, 0x%02x",    1, ld_l_d8 },
    { "CPL",             0, NULL },

    // 0x30
    { "JR NC, %hhd",     1, jr_nc },
    { "LD SP, 0x%04x",   2, NULL },
    { "LD (HL-), A",     0, ld_hlm_a },
    { "INC SP",          0, NULL },
    { "INC (HL)",        0, inc_mem_hl },
    { "DEC (HL)",        0, dec_mem_hl },
    { "LD (HL), 0x%02x", 1, ld_hl_d8 },
    { "SCF",             0, NULL },
    { "JR C, %hhd",      1, jr_c },
    { "ADD HL, SP",      0, NULL },
    { "LD A, (HL-)",     0, ld_a_hlm },
    { "DEC SP",          0, NULL },
    { "INC A",           0, inc_a },
    { "DEC A",           0, dec_a },
    { "LD A, 0x%02x",    1, ld_a_d8 },
    { "CCF",             0, NULL },

    // 0x40
    { "LD B, B",         0, NULL },
    { "LD B, C",         0, NULL },
    { "LD B, D",         0, NULL },
    { "LD B, E",         0, NULL },
    { "LD B, H",         0, NULL },
    { "LD B, L",         0, NULL },
    { "LD B, (HL)",      0, NULL },
    { "LD B, A",         0, NULL },
    { "LD C, B",         0, NULL },
    { "LD C, C",         0, NULL },
    { "LD C, D",         0, NULL },
    { "LD C, E",         0, NULL },
    { "LD C, H",         0, NULL },
    { "LD C, L",         0, NULL },
    { "LD C, (HL)",      0, NULL },
    { "LD C, A",         0, NULL },

    // 0x50
    { "LD D, B",         0, NULL },
    { "LD D, C",         0, NULL },
    { "LD D, D",         0, NULL },
    { "LD D, E",         0, NULL },
    { "LD D, H",         0, NULL },
    { "LD D, L",         0, NULL },
    { "LD D, (HL)",      0, NULL },
    { "LD D, A",         0, NULL },
    { "LD E, B",         0, NULL },
    { "LD E, C",         0, NULL },
    { "LD E, D",         0, NULL },
    { "LD E, E",         0, NULL },
    { "LD E, H",         0, NULL },
    { "LD E, L",         0, NULL },
    { "LD E, (HL)",      0, NULL },
    { "LD E, A",         0, NULL },

    // 0x60
    { "LD H, B",         0, NULL },
    { "LD H, C",         0, NULL },
    { "LD H, D",         0, NULL },
    { "LD H, E",         0, NULL },
    { "LD H, H",         0, NULL },
    { "LD H, L",         0, NULL },
    { "LD H, (HL)",      0, NULL },
    { "LD H, A",         0, NULL },
    { "LD L, B",         0, NULL },
    { "LD L, C",         0, NULL },
    { "LD L, D",         0, NULL },
    { "LD L, E",         0, NULL },
    { "LD L, H",         0, NULL },
    { "LD L, L",         0, NULL },
    { "LD L, (HL)",      0, NULL },
    { "LD L, A",         0, NULL },

    // 0x70
    { "LD (HL), B",      0, NULL },
    { "LD (HL), C",      0, NULL },
    { "LD (HL), D",      0, NULL },
    { "LD (HL), E",      0, NULL },
    { "LD (HL), H",      0, NULL },
    { "LD (HL), L",      0, NULL },
    { "HALT",            0, NULL },
    { "LD (HL), A",      0, NULL },
    { "LD A, B",         0, NULL },
    { "LD A, C",         0, NULL },
    { "LD A, D",         0, NULL },
    { "LD A, E",         0, NULL },
    { "LD A, H",         0, NULL },
    { "LD A, L",         0, NULL },
    { "LD A, (HL)",      0, NULL },
    { "LD A, A",         0, NULL },

    // 0x80
    { "ADD A, B",        0, NULL },
    { "ADD A, C",        0, NULL },
    { "ADD A, D",        0, NULL },
    { "ADD A, E",        0, NULL },
    { "ADD A, H",        0, NULL },
    { "ADD A, L",        0, NULL },
    { "ADD A, (HL)",     0, NULL },
    { "ADD A, A",        0, NULL },
    { "ADC A, B",        0, NULL },
    { "ADC A, C",        0, NULL },
    { "ADC A, D",        0, NULL },
    { "ADC A, E",        0, NULL },
    { "ADC A, H",        0, NULL },
    { "ADC A, L",        0, NULL },
    { "ADC A, (HL)",     0, NULL },
    { "ADC A, A",        0, NULL },

    // 0x90
    { "SUB B",           0, NULL },
    { "SUB C",           0, NULL },
    { "SUB D",           0, NULL },
    { "SUB E",           0, NULL },
    { "SUB H",           0, NULL },
    { "SUB L",           0, NULL },
    { "SUB (HL)",        0, NULL },
    { "SUB A",           0, NULL },
    { "SBC A, B",        0, NULL },
    { "SBC A, C",        0, NULL },
    { "SBC A, D",        0, NULL },
    { "SBC A, E",        0, NULL },
    { "SBC A, H",        0, NULL },
    { "SBC A, L",        0, NULL },
    { "SBC A, (HL)",     0, NULL },
    { "SBC A, A",        0, NULL },

    // 0xa0
    { "AND B",           0, NULL },
    { "AND C",           0, NULL },
    { "AND D",           0, NULL },
    { "AND E",           0, NULL },
    { "AND H",           0, NULL },
    { "AND L",           0, NULL },
    { "AND (HL)",        0, NULL },
    { "AND A",           0, NULL },
    { "XOR B",           0, xor_b },
    { "XOR C",           0, xor_c },
    { "XOR D",           0, xor_d },
    { "XOR E",           0, xor_e },
    { "XOR H",           0, xor_h },
    { "XOR L",           0, xor_l },
    { "XOR (HL)",        0, xor_hl },
    { "XOR A",           0, xor_a },

    // 0xb0
    { "OR B",            0, NULL },
    { "OR C",            0, NULL },
    { "OR D",            0, NULL },
    { "OR E",            0, NULL },
    { "OR H",            0, NULL },
    { "OR L",            0, NULL },
    { "OR (HL)",         0, NULL },
    { "OR A",            0, NULL },
    { "CP B",            0, cp_b },
    { "CP C",            0, cp_c },
    { "CP D",            0, cp_d },
    { "CP E",            0, cp_e },
    { "CP H",            0, cp_h },
    { "CP L",            0, cp_l },
    { "CP (HL)",         0, cp_mem_hl },
    { "CP A",            0, cp_a },

    // 0xc0
    { "RET NZ",          0, NULL },
    { "POP BC",          0, NULL },
    { "JP NZ, 0x%04x",   2, NULL },
    { "JP 0x%04x",       2, jp_a16 },
    { "CALL NZ, 0x%04x", 2, NULL },
    { "PUSH BC",         0, NULL },
    { "ADD A, 0x%02x",   1, NULL },
    { "RST 0x00",        0, rst00 },
    { "RET Z",           0, NULL },
    { "RET",             0, NULL },
    { "JP Z, 0x%04x",    2, NULL },
    { "<prefix cb>",     0, NULL },
    { "CALL Z, 0x%04x",  2, NULL },
    { "CALL 0x%04x",     2, NULL },
    { "ADC A, 0x%02x",   1, NULL },
    { "RST 0x08",        0, rst08 },

    // 0xd0
    { "RET NC",          0, NULL },
    { "POP DE",          0, NULL },
    { "JP NC, 0x%04x",   2, NULL },
    { "<undocumented>",  0, NULL },
    { "CALL NC, 0x%04x", 2, NULL },
    { "PUSH DE",         0, NULL },
    { "SUB 0x%02x",      1, NULL },
    { "RST 0x10",        0, rst10 },
    { "RET C",           0, NULL },
    { "RETI",            0, NULL },
    { "JP C, 0x%04x",    2, NULL },
    { "<undocumented>",  0, NULL },
    { "CALL C, 0x%04x",  2, NULL },
    { "<undocumented>",  0, NULL },
    { "SBC A, 0x%02x",   1, NULL },
    { "RST 0x18",        0, rst18 },

    // 0xe0
    { "LDH (0x%02x), A", 1, ldh_a8_a },
    { "POP HL",          0, NULL },
    { "LD (C), A",       0, NULL },
    { "<undocumented>",  0, NULL },
    { "<undocumented>",  0, NULL },
    { "PUSH HL",         0, NULL },
    { "AND 0x%02x",      1, NULL },
    { "RST 0x20",        0, rst20 },
    { "ADD SP, %hhd",    1, NULL },
    { "JP (HL)",         0, NULL },
    { "LD (0x%04x), A",  2, NULL },
    { "<undocumented>",  0, NULL },
    { "<undocumented>",  0, NULL },
    { "<undocumented>",  0, NULL },
    { "XOR A, 0x%02x",   1, NULL },
    { "RST 0x28",        0, rst28 },

    // 0xf0
    { "LDH A, (0x%02x)", 1, ldh_a_a8 },
    { "POP AF",          0, NULL },
    { "LD A, (C)",       0, NULL },
    { "DI",              0, di },
    { "<undocumented>",  0, NULL },
    { "PUSH AF",         0, NULL },
    { "OR 0x%02x",       1, NULL },
    { "RST 0x30",        0, rst30 },
    { "LD HL, SP%+d",    1, NULL },
    { "LD SP, HL",       0, NULL },
    { "LD A, (0x%04x)",  2, NULL },
    { "EI",              0, ei },
    { "<undocumented>",  0, NULL },
    { "<undocumented>",  0, NULL },
    { "CP A, 0x%02x",    1, cp_d8 },
    { "RST 0x38",        0, rst38 },
};


const struct Instruction cbInstructions[256] = {
    // 0x00
    { "RLC B",      0, NULL },
    { "RLC C",      0, NULL },
    { "RLC D",      0, NULL },
    { "RLC E",      0, NULL },
    { "RLC H",      0, NULL },
    { "RLC L",      0, NULL },
    { "RLC (HL)",   0, NULL },
    { "RLC A",      0, NULL },
    { "RRC B",      0, NULL },
    { "RRC C",      0, NULL },
    { "RRC D",      0, NULL },
    { "RRC E",      0, NULL },
    { "RRC H",      0, NULL },
    { "RRC L",      0, NULL },
    { "RRC (HL)",   0, NULL },
    { "RRC A",      0, NULL },

    // 0x10
    { "RL B",       0, NULL },
    { "RL C",       0, NULL },
    { "RL D",       0, NULL },
    { "RL E",       0, NULL },
    { "RL H",       0, NULL },
    { "RL L",       0, NULL },
    { "RL (HL)",    0, NULL },
    { "RL A",       0, NULL },
    { "RR B",       0, NULL },
    { "RR C",       0, NULL },
    { "RR D",       0, NULL },
    { "RR E",       0, NULL },
    { "RR H",       0, NULL },
    { "RR L",       0, NULL },
    { "RR (HL)",    0, NULL },
    { "RR A",       0, NULL },

    // 0x20
    { "SLA B",      0, NULL },
    { "SLA C",      0, NULL },
    { "SLA D",      0, NULL },
    { "SLA E",      0, NULL },
    { "SLA H",      0, NULL },
    { "SLA L",      0, NULL },
    { "SLA (HL)",   0, NULL },
    { "SLA A",      0, NULL },
    { "SRA B",      0, NULL },
    { "SRA C",      0, NULL },
    { "SRA D",      0, NULL },
    { "SRA E",      0, NULL },
    { "SRA H",      0, NULL },
    { "SRA L",      0, NULL },
    { "SRA (HL)",   0, NULL },
    { "SRA A",      0, NULL },

    // 0x30
    { "SWAP B",     0, NULL },
    { "SWAP C",     0, NULL },
    { "SWAP D",     0, NULL },
    { "SWAP E",     0, NULL },
    { "SWAP H",     0, NULL },
    { "SWAP L",     0, NULL },
    { "SWAP (HL)",  0, NULL },
    { "SWAP A",     0, NULL },
    { "SRL B",      0, NULL },
    { "SRL C",      0, NULL },
    { "SRL D",      0, NULL },
    { "SRL E",      0, NULL },
    { "SRL H",      0, NULL },
    { "SRL L",      0, NULL },
    { "SRL (HL)",   0, NULL },
    { "SRL A",      0, NULL },

    // 0x40
    { "BIT 0 B",      0, NULL },
    { "BIT 0 C",      0, NULL },
    { "BIT 0 D",      0, NULL },
    { "BIT 0 E",      0, NULL },
    { "BIT 0 H",      0, NULL },
    { "BIT 0 L",      0, NULL },
    { "BIT 0 (HL)",   0, NULL },
    { "BIT 0 A",      0, NULL },
    { "BIT 1 B",      0, NULL },
    { "BIT 1 C",      0, NULL },
    { "BIT 1 D",      0, NULL },
    { "BIT 1 E",      0, NULL },
    { "BIT 1 H",      0, NULL },
    { "BIT 1 L",      0, NULL },
    { "BIT 1 (HL)",   0, NULL },
    { "BIT 1 A",      0, NULL },

    // 0x50
    { "BIT 2 B",      0, NULL },
    { "BIT 2 C",      0, NULL },
    { "BIT 2 D",      0, NULL },
    { "BIT 2 E",      0, NULL },
    { "BIT 2 H",      0, NULL },
    { "BIT 2 L",      0, NULL },
    { "BIT 2 (HL)",   0, NULL },
    { "BIT 2 A",      0, NULL },
    { "BIT 3 B",      0, NULL },
    { "BIT 3 C",      0, NULL },
    { "BIT 3 D",      0, NULL },
    { "BIT 3 E",      0, NULL },
    { "BIT 3 H",      0, NULL },
    { "BIT 3 L",      0, NULL },
    { "BIT 3 (HL)",   0, NULL },
    { "BIT 3 A",      0, NULL },

    // 0x60
    { "BIT 4 B",      0, NULL },
    { "BIT 4 C",      0, NULL },
    { "BIT 4 D",      0, NULL },
    { "BIT 4 E",      0, NULL },
    { "BIT 4 H",      0, NULL },
    { "BIT 4 L",      0, NULL },
    { "BIT 4 (HL)",   0, NULL },
    { "BIT 4 A",      0, NULL },
    { "BIT 5 B",      0, NULL },
    { "BIT 5 C",      0, NULL },
    { "BIT 5 D",      0, NULL },
    { "BIT 5 E",      0, NULL },
    { "BIT 5 H",      0, NULL },
    { "BIT 5 L",      0, NULL },
    { "BIT 5 (HL)",   0, NULL },
    { "BIT 5 A",      0, NULL },

    // 0x70
    { "BIT 6 B",      0, NULL },
    { "BIT 6 C",      0, NULL },
    { "BIT 6 D",      0, NULL },
    { "BIT 6 E",      0, NULL },
    { "BIT 6 H",      0, NULL },
    { "BIT 6 L",      0, NULL },
    { "BIT 6 (HL)",   0, NULL },
    { "BIT 6 A",      0, NULL },
    { "BIT 7 B",      0, NULL },
    { "BIT 7 C",      0, NULL },
    { "BIT 7 D",      0, NULL },
    { "BIT 7 E",      0, NULL },
    { "BIT 7 H",      0, NULL },
    { "BIT 7 L",      0, NULL },
    { "BIT 7 (HL)",   0, NULL },
    { "BIT 7 A",      0, NULL },

    // 0x80
    { "RES 0 B",      0, NULL },
    { "RES 0 C",      0, NULL },
    { "RES 0 D",      0, NULL },
    { "RES 0 E",      0, NULL },
    { "RES 0 H",      0, NULL },
    { "RES 0 L",      0, NULL },
    { "RES 0 (HL)",   0, NULL },
    { "RES 0 A",      0, NULL },
    { "RES 1 B",      0, NULL },
    { "RES 1 C",      0, NULL },
    { "RES 1 D",      0, NULL },
    { "RES 1 E",      0, NULL },
    { "RES 1 H",      0, NULL },
    { "RES 1 L",      0, NULL },
    { "RES 1 (HL)",   0, NULL },
    { "RES 1 A",      0, NULL },

    // 0x90
    { "RES 2 B",      0, NULL },
    { "RES 2 C",      0, NULL },
    { "RES 2 D",      0, NULL },
    { "RES 2 E",      0, NULL },
    { "RES 2 H",      0, NULL },
    { "RES 2 L",      0, NULL },
    { "RES 2 (HL)",   0, NULL },
    { "RES 2 A",      0, NULL },
    { "RES 3 B",      0, NULL },
    { "RES 3 C",      0, NULL },
    { "RES 3 D",      0, NULL },
    { "RES 3 E",      0, NULL },
    { "RES 3 H",      0, NULL },
    { "RES 3 L",      0, NULL },
    { "RES 3 (HL)",   0, NULL },
    { "RES 3 A",      0, NULL },

    // 0xa0
    { "RES 4 B",      0, NULL },
    { "RES 4 C",      0, NULL },
    { "RES 4 D",      0, NULL },
    { "RES 4 E",      0, NULL },
    { "RES 4 H",      0, NULL },
    { "RES 4 L",      0, NULL },
    { "RES 4 (HL)",   0, NULL },
    { "RES 4 A",      0, NULL },
    { "RES 5 B",      0, NULL },
    { "RES 5 C",      0, NULL },
    { "RES 5 D",      0, NULL },
    { "RES 5 E",      0, NULL },
    { "RES 5 H",      0, NULL },
    { "RES 5 L",      0, NULL },
    { "RES 5 (HL)",   0, NULL },
    { "RES 5 A",      0, NULL },

    // 0xb0
    { "RES 6 B",      0, NULL },
    { "RES 6 C",      0, NULL },
    { "RES 6 D",      0, NULL },
    { "RES 6 E",      0, NULL },
    { "RES 6 H",      0, NULL },
    { "RES 6 L",      0, NULL },
    { "RES 6 (HL)",   0, NULL },
    { "RES 6 A",      0, NULL },
    { "RES 7 B",      0, NULL },
    { "RES 7 C",      0, NULL },
    { "RES 7 D",      0, NULL },
    { "RES 7 E",      0, NULL },
    { "RES 7 H",      0, NULL },
    { "RES 7 L",      0, NULL },
    { "RES 7 (HL)",   0, NULL },
    { "RES 7 A",      0, NULL },

    // 0xc0
    { "SET 0 B",      0, NULL },
    { "SET 0 C",      0, NULL },
    { "SET 0 D",      0, NULL },
    { "SET 0 E",      0, NULL },
    { "SET 0 H",      0, NULL },
    { "SET 0 L",      0, NULL },
    { "SET 0 (HL)",   0, NULL },
    { "SET 0 A",      0, NULL },
    { "SET 1 B",      0, NULL },
    { "SET 1 C",      0, NULL },
    { "SET 1 D",      0, NULL },
    { "SET 1 E",      0, NULL },
    { "SET 1 H",      0, NULL },
    { "SET 1 L",      0, NULL },
    { "SET 1 (HL)",   0, NULL },
    { "SET 1 A",      0, NULL },

    // 0xd0
    { "SET 2 B",      0, NULL },
    { "SET 2 C",      0, NULL },
    { "SET 2 D",      0, NULL },
    { "SET 2 E",      0, NULL },
    { "SET 2 H",      0, NULL },
    { "SET 2 L",      0, NULL },
    { "SET 2 (HL)",   0, NULL },
    { "SET 2 A",      0, NULL },
    { "SET 3 B",      0, NULL },
    { "SET 3 C",      0, NULL },
    { "SET 3 D",      0, NULL },
    { "SET 3 E",      0, NULL },
    { "SET 3 H",      0, NULL },
    { "SET 3 L",      0, NULL },
    { "SET 3 (HL)",   0, NULL },
    { "SET 3 A",      0, NULL },

    // 0xe0
    { "SET 4 B",      0, NULL },
    { "SET 4 C",      0, NULL },
    { "SET 4 D",      0, NULL },
    { "SET 4 E",      0, NULL },
    { "SET 4 H",      0, NULL },
    { "SET 4 L",      0, NULL },
    { "SET 4 (HL)",   0, NULL },
    { "SET 4 A",      0, NULL },
    { "SET 5 B",      0, NULL },
    { "SET 5 C",      0, NULL },
    { "SET 5 D",      0, NULL },
    { "SET 5 E",      0, NULL },
    { "SET 5 H",      0, NULL },
    { "SET 5 L",      0, NULL },
    { "SET 5 (HL)",   0, NULL },
    { "SET 5 A",      0, NULL },

    // 0xf0
    { "SET 6 B",      0, NULL },
    { "SET 6 C",      0, NULL },
    { "SET 6 D",      0, NULL },
    { "SET 6 E",      0, NULL },
    { "SET 6 H",      0, NULL },
    { "SET 6 L",      0, NULL },
    { "SET 6 (HL)",   0, NULL },
    { "SET 6 A",      0, NULL },
    { "SET 7 B",      0, NULL },
    { "SET 7 C",      0, NULL },
    { "SET 7 D",      0, NULL },
    { "SET 7 E",      0, NULL },
    { "SET 7 H",      0, NULL },
    { "SET 7 L",      0, NULL },
    { "SET 7 (HL)",   0, NULL },
    { "SET 7 A",      0, NULL },
};
