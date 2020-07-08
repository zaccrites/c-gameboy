
#include "cpu_instructions.h"
#include "cpu.h"
#include "memory.h"

// TODO: Reorganize this file. Group and rename functions as appropriate.


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


static void push_bc(struct Cpu *cpu) { cpu_push_dword(cpu, cpu_read_double_reg(cpu, CPU_DOUBLE_REG_BC)); }
static void push_de(struct Cpu *cpu) { cpu_push_dword(cpu, cpu_read_double_reg(cpu, CPU_DOUBLE_REG_DE)); }
static void push_hl(struct Cpu *cpu) { cpu_push_dword(cpu, cpu_read_double_reg(cpu, CPU_DOUBLE_REG_HL)); }
static void push_af(struct Cpu *cpu) { cpu_push_dword(cpu, cpu_read_double_reg(cpu, CPU_DOUBLE_REG_AF)); }

static void pop_bc(struct Cpu *cpu) { cpu_write_double_reg(cpu, CPU_DOUBLE_REG_BC, cpu_pop_dword(cpu)); }
static void pop_de(struct Cpu *cpu) { cpu_write_double_reg(cpu, CPU_DOUBLE_REG_DE, cpu_pop_dword(cpu)); }
static void pop_hl(struct Cpu *cpu) { cpu_write_double_reg(cpu, CPU_DOUBLE_REG_HL, cpu_pop_dword(cpu)); }
static void pop_af(struct Cpu *cpu) { cpu_write_double_reg(cpu, CPU_DOUBLE_REG_AF, cpu_pop_dword(cpu)); }


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


static void jp_hl(struct Cpu *cpu)
{
    cpu->pc = cpu_read_double_reg(cpu, CPU_DOUBLE_REG_HL);
}

static void _jp(struct Cpu *cpu, bool condition)
{
    uint16_t address = imm_dword(cpu);
    if (condition)
    {
        cpu->pc = address;
    }
}
static void jp_a16(struct Cpu *cpu) { _jp(cpu, true); }
static void jp_z_a16(struct Cpu *cpu) { _jp(cpu, cpu->flags.zero); }
static void jp_nz_a16(struct Cpu *cpu) { _jp(cpu, ! cpu->flags.zero); }
static void jp_c_a16(struct Cpu *cpu) { _jp(cpu, cpu->flags.carry); }
static void jp_nc_a16(struct Cpu *cpu) { _jp(cpu, ! cpu->flags.carry); }



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
    cpu_push_dword(cpu, cpu->pc);
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


static void ld_mem_a16_a(struct Cpu *cpu)
{
    uint16_t address = imm_dword(cpu);
    memory_write_word(cpu->memory, address, cpu->registers.a);
}
static void ld_a_mem_a16(struct Cpu *cpu)
{
    uint16_t address = imm_dword(cpu);
    cpu->registers.a = memory_read_word(cpu->memory, address);
}

static void ld_a_d8(struct Cpu *cpu) { cpu->registers.a = imm_word(cpu); }
static void ld_b_d8(struct Cpu *cpu) { cpu->registers.b = imm_word(cpu); }
static void ld_c_d8(struct Cpu *cpu) { cpu->registers.c = imm_word(cpu); }
static void ld_d_d8(struct Cpu *cpu) { cpu->registers.d = imm_word(cpu); }
static void ld_e_d8(struct Cpu *cpu) { cpu->registers.e = imm_word(cpu); }
static void ld_h_d8(struct Cpu *cpu) { cpu->registers.h = imm_word(cpu); }
static void ld_l_d8(struct Cpu *cpu) { cpu->registers.l = imm_word(cpu); }
static void ld_mem_hl_d8(struct Cpu *cpu) { write_mem_at_hl(cpu, imm_word(cpu)); }

static void ld_mem_hlm_a(struct Cpu *cpu)
{
    write_mem_at_hl(cpu, cpu->registers.a);
    modify_double_reg(cpu, CPU_DOUBLE_REG_HL, -1);
}
static void ld_mem_hlp_a(struct Cpu *cpu)
{
    write_mem_at_hl(cpu, cpu->registers.a);
    modify_double_reg(cpu, CPU_DOUBLE_REG_HL, +1);
}
static void ld_a_mem_hlm(struct Cpu *cpu)
{
    cpu->registers.a = read_mem_at_hl(cpu);
    modify_double_reg(cpu, CPU_DOUBLE_REG_HL, -1);
}
static void ld_a_mem_hlp(struct Cpu *cpu)
{
    cpu->registers.a = read_mem_at_hl(cpu);
    modify_double_reg(cpu, CPU_DOUBLE_REG_HL, +1);
}

static void ld_mem_bc_a(struct Cpu *cpu)
{
    uint16_t address = cpu_read_double_reg(cpu, CPU_DOUBLE_REG_BC);
    memory_write_word(cpu->memory, address, cpu->registers.a);
}
static void ld_mem_de_a(struct Cpu *cpu)
{
    uint16_t address = cpu_read_double_reg(cpu, CPU_DOUBLE_REG_DE);
    memory_write_word(cpu->memory, address, cpu->registers.a);
}
static void ld_mem_a16_sp(struct Cpu *cpu)
{
    memory_write_dword(cpu->memory, imm_dword(cpu), cpu->sp);
}

static void ld_a_mem_bc(struct Cpu *cpu)
{
    uint16_t address = cpu_read_double_reg(cpu, CPU_DOUBLE_REG_BC);
    cpu->registers.a = memory_read_word(cpu->memory, address);
}
static void ld_a_mem_de(struct Cpu *cpu)
{
    uint16_t address = cpu_read_double_reg(cpu, CPU_DOUBLE_REG_DE);
    cpu->registers.a = memory_read_word(cpu->memory, address);
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

static void ld_mem_c_a(struct Cpu *cpu)
{
    uint16_t address = 0xff00 | cpu->registers.c;
    memory_write_word(cpu->memory, address, cpu->registers.a);
}


static void ld_sp_d16(struct Cpu *cpu) { cpu->sp = imm_dword(cpu); }
static void ld_bc_d16(struct Cpu *cpu) { cpu_write_double_reg(cpu, CPU_DOUBLE_REG_BC, imm_dword(cpu)); }
static void ld_de_d16(struct Cpu *cpu) { cpu_write_double_reg(cpu, CPU_DOUBLE_REG_DE, imm_dword(cpu)); }
static void ld_hl_d16(struct Cpu *cpu) { cpu_write_double_reg(cpu, CPU_DOUBLE_REG_HL, imm_dword(cpu)); }

static void ld_sp_hl(struct Cpu *cpu) { cpu->sp = cpu_read_double_reg(cpu, CPU_DOUBLE_REG_HL); }



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



static void call_a16(struct Cpu *cpu)
{
    uint16_t address = imm_dword(cpu);
    cpu_push_dword(cpu, cpu->pc);
    cpu->pc = address;
}


static void _ret(struct Cpu *cpu, bool condition)
{
    if (condition)
    {
        cpu->pc = cpu_pop_dword(cpu);
    }
}
static void ret(struct Cpu *cpu) { _ret(cpu, true); }
static void ret_z(struct Cpu *cpu) { _ret(cpu, cpu->flags.zero); }
static void ret_nz(struct Cpu *cpu) { _ret(cpu, ! cpu->flags.zero); }
static void ret_c(struct Cpu *cpu) { _ret(cpu, cpu->flags.carry); }
static void ret_nc(struct Cpu *cpu) { _ret(cpu, ! cpu->flags.carry); }
static void reti(struct Cpu *cpu) { ret(cpu); cpu->ime = true; }


static void inc_bc(struct Cpu *cpu) { modify_double_reg(cpu, CPU_DOUBLE_REG_BC, +1); }
static void inc_de(struct Cpu *cpu) { modify_double_reg(cpu, CPU_DOUBLE_REG_DE, +1); }
static void inc_hl(struct Cpu *cpu) { modify_double_reg(cpu, CPU_DOUBLE_REG_HL, +1); }
static void inc_sp(struct Cpu *cpu) { cpu->sp += 1; }

static void dec_bc(struct Cpu *cpu) { modify_double_reg(cpu, CPU_DOUBLE_REG_BC, -1); }
static void dec_de(struct Cpu *cpu) { modify_double_reg(cpu, CPU_DOUBLE_REG_DE, -1); }
static void dec_hl(struct Cpu *cpu) { modify_double_reg(cpu, CPU_DOUBLE_REG_HL, -1); }
static void dec_sp(struct Cpu *cpu) { cpu->sp -= 1; }



static void ld_a_a(struct Cpu *cpu) { cpu->registers.a = cpu->registers.a; }
static void ld_a_b(struct Cpu *cpu) { cpu->registers.a = cpu->registers.b; }
static void ld_a_c(struct Cpu *cpu) { cpu->registers.a = cpu->registers.c; }
static void ld_a_d(struct Cpu *cpu) { cpu->registers.a = cpu->registers.d; }
static void ld_a_e(struct Cpu *cpu) { cpu->registers.a = cpu->registers.e; }
static void ld_a_h(struct Cpu *cpu) { cpu->registers.a = cpu->registers.h; }
static void ld_a_l(struct Cpu *cpu) { cpu->registers.a = cpu->registers.l; }
static void ld_a_hl(struct Cpu *cpu) { cpu->registers.a = read_mem_at_hl(cpu); }

static void ld_b_a(struct Cpu *cpu) { cpu->registers.b = cpu->registers.a; }
static void ld_b_b(struct Cpu *cpu) { cpu->registers.b = cpu->registers.b; }
static void ld_b_c(struct Cpu *cpu) { cpu->registers.b = cpu->registers.c; }
static void ld_b_d(struct Cpu *cpu) { cpu->registers.b = cpu->registers.d; }
static void ld_b_e(struct Cpu *cpu) { cpu->registers.b = cpu->registers.e; }
static void ld_b_h(struct Cpu *cpu) { cpu->registers.b = cpu->registers.h; }
static void ld_b_l(struct Cpu *cpu) { cpu->registers.b = cpu->registers.l; }
static void ld_b_hl(struct Cpu *cpu) { cpu->registers.b = read_mem_at_hl(cpu); }

static void ld_c_a(struct Cpu *cpu) { cpu->registers.c = cpu->registers.a; }
static void ld_c_b(struct Cpu *cpu) { cpu->registers.c = cpu->registers.b; }
static void ld_c_c(struct Cpu *cpu) { cpu->registers.c = cpu->registers.c; }
static void ld_c_d(struct Cpu *cpu) { cpu->registers.c = cpu->registers.d; }
static void ld_c_e(struct Cpu *cpu) { cpu->registers.c = cpu->registers.e; }
static void ld_c_h(struct Cpu *cpu) { cpu->registers.c = cpu->registers.h; }
static void ld_c_l(struct Cpu *cpu) { cpu->registers.c = cpu->registers.l; }
static void ld_c_hl(struct Cpu *cpu) { cpu->registers.c = read_mem_at_hl(cpu); }

static void ld_d_a(struct Cpu *cpu) { cpu->registers.d = cpu->registers.a; }
static void ld_d_b(struct Cpu *cpu) { cpu->registers.d = cpu->registers.b; }
static void ld_d_c(struct Cpu *cpu) { cpu->registers.d = cpu->registers.c; }
static void ld_d_d(struct Cpu *cpu) { cpu->registers.d = cpu->registers.d; }
static void ld_d_e(struct Cpu *cpu) { cpu->registers.d = cpu->registers.e; }
static void ld_d_h(struct Cpu *cpu) { cpu->registers.d = cpu->registers.h; }
static void ld_d_l(struct Cpu *cpu) { cpu->registers.d = cpu->registers.l; }
static void ld_d_hl(struct Cpu *cpu) { cpu->registers.d = read_mem_at_hl(cpu); }

static void ld_e_a(struct Cpu *cpu) { cpu->registers.e = cpu->registers.a; }
static void ld_e_b(struct Cpu *cpu) { cpu->registers.e = cpu->registers.b; }
static void ld_e_c(struct Cpu *cpu) { cpu->registers.e = cpu->registers.c; }
static void ld_e_d(struct Cpu *cpu) { cpu->registers.e = cpu->registers.d; }
static void ld_e_e(struct Cpu *cpu) { cpu->registers.e = cpu->registers.e; }
static void ld_e_h(struct Cpu *cpu) { cpu->registers.e = cpu->registers.h; }
static void ld_e_l(struct Cpu *cpu) { cpu->registers.e = cpu->registers.l; }
static void ld_e_hl(struct Cpu *cpu) { cpu->registers.e = read_mem_at_hl(cpu); }

static void ld_h_a(struct Cpu *cpu) { cpu->registers.h = cpu->registers.a; }
static void ld_h_b(struct Cpu *cpu) { cpu->registers.h = cpu->registers.b; }
static void ld_h_c(struct Cpu *cpu) { cpu->registers.h = cpu->registers.c; }
static void ld_h_d(struct Cpu *cpu) { cpu->registers.h = cpu->registers.d; }
static void ld_h_e(struct Cpu *cpu) { cpu->registers.h = cpu->registers.e; }
static void ld_h_h(struct Cpu *cpu) { cpu->registers.h = cpu->registers.h; }
static void ld_h_l(struct Cpu *cpu) { cpu->registers.h = cpu->registers.l; }
static void ld_h_hl(struct Cpu *cpu) { cpu->registers.h = read_mem_at_hl(cpu); }

static void ld_l_a(struct Cpu *cpu) { cpu->registers.l = cpu->registers.a; }
static void ld_l_b(struct Cpu *cpu) { cpu->registers.l = cpu->registers.b; }
static void ld_l_c(struct Cpu *cpu) { cpu->registers.l = cpu->registers.c; }
static void ld_l_d(struct Cpu *cpu) { cpu->registers.l = cpu->registers.d; }
static void ld_l_e(struct Cpu *cpu) { cpu->registers.l = cpu->registers.e; }
static void ld_l_h(struct Cpu *cpu) { cpu->registers.l = cpu->registers.h; }
static void ld_l_l(struct Cpu *cpu) { cpu->registers.l = cpu->registers.l; }
static void ld_l_hl(struct Cpu *cpu) { cpu->registers.l = read_mem_at_hl(cpu); }

static void ld_hl_a(struct Cpu *cpu) { write_mem_at_hl(cpu, cpu->registers.a); }
static void ld_hl_b(struct Cpu *cpu) { write_mem_at_hl(cpu, cpu->registers.b); }
static void ld_hl_c(struct Cpu *cpu) { write_mem_at_hl(cpu, cpu->registers.c); }
static void ld_hl_d(struct Cpu *cpu) { write_mem_at_hl(cpu, cpu->registers.d); }
static void ld_hl_e(struct Cpu *cpu) { write_mem_at_hl(cpu, cpu->registers.e); }
static void ld_hl_h(struct Cpu *cpu) { write_mem_at_hl(cpu, cpu->registers.h); }
static void ld_hl_l(struct Cpu *cpu) { write_mem_at_hl(cpu, cpu->registers.l); }


static void and(struct Cpu *cpu, uint8_t value)
{
    cpu->registers.a &= value;
    cpu->flags.zero = cpu->registers.a == 0;
    cpu->flags.negative = false;
    cpu->flags.halfCarry = true;
    cpu->flags.carry = false;
}
static void and_a(struct Cpu *cpu) { and(cpu, cpu->registers.a); }
static void and_b(struct Cpu *cpu) { and(cpu, cpu->registers.b); }
static void and_c(struct Cpu *cpu) { and(cpu, cpu->registers.c); }
static void and_d(struct Cpu *cpu) { and(cpu, cpu->registers.d); }
static void and_e(struct Cpu *cpu) { and(cpu, cpu->registers.e); }
static void and_h(struct Cpu *cpu) { and(cpu, cpu->registers.h); }
static void and_l(struct Cpu *cpu) { and(cpu, cpu->registers.l); }
static void and_hl(struct Cpu *cpu) { and(cpu, read_mem_at_hl(cpu)); }
static void and_d8(struct Cpu *cpu) { and(cpu, imm_word(cpu)); }


static void or(struct Cpu *cpu, uint8_t value)
{
    cpu->registers.a |= value;
    cpu->flags.zero = cpu->registers.a == 0;
    cpu->flags.negative = false;
    cpu->flags.halfCarry = false;
    cpu->flags.carry = false;
}
static void or_a(struct Cpu *cpu) { or(cpu, cpu->registers.a); }
static void or_b(struct Cpu *cpu) { or(cpu, cpu->registers.b); }
static void or_c(struct Cpu *cpu) { or(cpu, cpu->registers.c); }
static void or_d(struct Cpu *cpu) { or(cpu, cpu->registers.d); }
static void or_e(struct Cpu *cpu) { or(cpu, cpu->registers.e); }
static void or_h(struct Cpu *cpu) { or(cpu, cpu->registers.h); }
static void or_l(struct Cpu *cpu) { or(cpu, cpu->registers.l); }
static void or_hl(struct Cpu *cpu) { or(cpu, read_mem_at_hl(cpu)); }
static void or_d8(struct Cpu *cpu) { or(cpu, imm_word(cpu)); }


static void add(struct Cpu *cpu, uint8_t value)
{
    uint8_t newValue = cpu->registers.a + value;
    cpu->flags.zero = newValue == 0;
    cpu->flags.negative = false;
    cpu->flags.halfCarry = (((cpu->registers.a & 0x0f) + (value & 0x0f)) & 0x10) != 0;
    cpu->flags.carry = 0xff - cpu->registers.a > value;
    cpu->registers.a = newValue;
}
static void add_a(struct Cpu *cpu) { add(cpu, cpu->registers.a); }
static void add_b(struct Cpu *cpu) { add(cpu, cpu->registers.b); }
static void add_c(struct Cpu *cpu) { add(cpu, cpu->registers.c); }
static void add_d(struct Cpu *cpu) { add(cpu, cpu->registers.d); }
static void add_e(struct Cpu *cpu) { add(cpu, cpu->registers.e); }
static void add_h(struct Cpu *cpu) { add(cpu, cpu->registers.h); }
static void add_l(struct Cpu *cpu) { add(cpu, cpu->registers.l); }
static void add_hl(struct Cpu *cpu) { add(cpu, read_mem_at_hl(cpu)); }
static void add_d8(struct Cpu *cpu) { add(cpu, imm_word(cpu)); }


static void adc(struct Cpu *cpu, uint8_t value)
{
    uint8_t newValue = cpu->registers.a + value + (cpu->flags.carry ? 1 : 0);
    cpu->flags.zero = newValue == 0;
    cpu->flags.negative = false;
    cpu->flags.halfCarry = (((cpu->registers.a & 0x0f) + (value & 0x0f)) & 0x10) != 0;
    cpu->flags.carry = 0xff - cpu->registers.a > value;
    cpu->registers.a = newValue;
}
static void adc_a(struct Cpu *cpu) { adc(cpu, cpu->registers.a); }
static void adc_b(struct Cpu *cpu) { adc(cpu, cpu->registers.b); }
static void adc_c(struct Cpu *cpu) { adc(cpu, cpu->registers.c); }
static void adc_d(struct Cpu *cpu) { adc(cpu, cpu->registers.d); }
static void adc_e(struct Cpu *cpu) { adc(cpu, cpu->registers.e); }
static void adc_h(struct Cpu *cpu) { adc(cpu, cpu->registers.h); }
static void adc_l(struct Cpu *cpu) { adc(cpu, cpu->registers.l); }
static void adc_hl(struct Cpu *cpu) { adc(cpu, read_mem_at_hl(cpu)); }
static void adc_d8(struct Cpu *cpu) { adc(cpu, imm_word(cpu)); }




static void sub(struct Cpu *cpu, uint8_t value)
{
    uint8_t newValue = cpu->registers.a - value;
    cpu->flags.zero = newValue == 0;
    cpu->flags.negative = false;
    cpu->flags.halfCarry = (cpu->registers.a & 0x0f) < (value & 0x0f);
    cpu->flags.carry = value > cpu->registers.a;
    cpu->registers.a = newValue;
}
static void sub_a(struct Cpu *cpu) { sub(cpu, cpu->registers.a); }
static void sub_b(struct Cpu *cpu) { sub(cpu, cpu->registers.b); }
static void sub_c(struct Cpu *cpu) { sub(cpu, cpu->registers.c); }
static void sub_d(struct Cpu *cpu) { sub(cpu, cpu->registers.d); }
static void sub_e(struct Cpu *cpu) { sub(cpu, cpu->registers.e); }
static void sub_h(struct Cpu *cpu) { sub(cpu, cpu->registers.h); }
static void sub_l(struct Cpu *cpu) { sub(cpu, cpu->registers.l); }
static void sub_hl(struct Cpu *cpu) { sub(cpu, read_mem_at_hl(cpu)); }
static void sub_d8(struct Cpu *cpu) { sub(cpu, imm_word(cpu)); }



static void add_hl_rr(struct Cpu *cpu, uint16_t value)
{
    uint16_t hl = cpu_read_double_reg(cpu, CPU_DOUBLE_REG_HL);
    uint16_t newValue = hl + value;
    // zero flag not affected
    cpu->flags.negative = false;
    cpu->flags.halfCarry = (((hl & 0xff) + (value & 0xff)) & 0x0100) != 0;
    cpu->flags.carry = 0xffff - hl > value;
    cpu_write_double_reg(cpu, CPU_DOUBLE_REG_HL, newValue);
}
static void add_hl_bc(struct Cpu *cpu) { add_hl_rr(cpu, cpu_read_double_reg(cpu, CPU_DOUBLE_REG_BC)); }
static void add_hl_de(struct Cpu *cpu) { add_hl_rr(cpu, cpu_read_double_reg(cpu, CPU_DOUBLE_REG_DE)); }
static void add_hl_hl(struct Cpu *cpu) { add_hl_rr(cpu, cpu_read_double_reg(cpu, CPU_DOUBLE_REG_HL)); }
static void add_hl_sp(struct Cpu *cpu) { add_hl_rr(cpu, cpu->sp); }




static void cpl(struct Cpu *cpu)
{
    cpu->registers.a = ~cpu->registers.a;
    // zero flag not affected
    cpu->flags.negative = true;
    cpu->flags.halfCarry = true;
    // carry flag not affected
}


static void daa(struct Cpu *cpu)
{
    if ((cpu->registers.a & 0x0f) > 0x09 || cpu->flags.halfCarry)
    {
        cpu->registers.a += 0x06;
    }

    bool doSecondAddition = (cpu->registers.a & 0xf0) > 0x90 || cpu->flags.carry;
    if (doSecondAddition)
    {
        cpu->registers.a += 0x60;
    }

    cpu->flags.zero = cpu->registers.a == 0;
    // negative flag not affected
    cpu->flags.halfCarry = false;
    cpu->flags.carry = doSecondAddition;
}





static uint8_t swap(struct Cpu *cpu, uint8_t value)
{
    uint8_t upper = value >> 4;
    uint8_t lower = value & 0x0f;
    uint8_t newValue = (lower << 4) | upper;
    cpu->flags.zero = newValue == 0;
    cpu->flags.negative = false;
    cpu->flags.halfCarry = false;
    cpu->flags.carry = false;
    return newValue;
}
static void swap_a(struct Cpu *cpu) { cpu->registers.a = swap(cpu, cpu->registers.a); }
static void swap_b(struct Cpu *cpu) { cpu->registers.b = swap(cpu, cpu->registers.b); }
static void swap_c(struct Cpu *cpu) { cpu->registers.c = swap(cpu, cpu->registers.c); }
static void swap_d(struct Cpu *cpu) { cpu->registers.d = swap(cpu, cpu->registers.d); }
static void swap_e(struct Cpu *cpu) { cpu->registers.e = swap(cpu, cpu->registers.e); }
static void swap_h(struct Cpu *cpu) { cpu->registers.h = swap(cpu, cpu->registers.h); }
static void swap_l(struct Cpu *cpu) { cpu->registers.l = swap(cpu, cpu->registers.l); }
static void swap_hl(struct Cpu *cpu) { write_mem_at_hl(cpu, swap(cpu, read_mem_at_hl(cpu))); }



static void bit(struct Cpu *cpu, uint8_t bitnum, uint8_t value)
{
    cpu->flags.zero = (value & (1 << bitnum)) == 0;
    cpu->flags.negative = false;
    cpu->flags.halfCarry = false;
    // carry flag not affected
}
static void bit_0_a(struct Cpu *cpu) { bit(cpu, 0, cpu->registers.a); }
static void bit_0_b(struct Cpu *cpu) { bit(cpu, 0, cpu->registers.b); }
static void bit_0_c(struct Cpu *cpu) { bit(cpu, 0, cpu->registers.c); }
static void bit_0_d(struct Cpu *cpu) { bit(cpu, 0, cpu->registers.d); }
static void bit_0_e(struct Cpu *cpu) { bit(cpu, 0, cpu->registers.e); }
static void bit_0_h(struct Cpu *cpu) { bit(cpu, 0, cpu->registers.h); }
static void bit_0_l(struct Cpu *cpu) { bit(cpu, 0, cpu->registers.l); }
static void bit_0_mem_hl(struct Cpu *cpu) { bit(cpu, 0, read_mem_at_hl(cpu)); }
//
static void bit_1_a(struct Cpu *cpu) { bit(cpu, 1, cpu->registers.a); }
static void bit_1_b(struct Cpu *cpu) { bit(cpu, 1, cpu->registers.b); }
static void bit_1_c(struct Cpu *cpu) { bit(cpu, 1, cpu->registers.c); }
static void bit_1_d(struct Cpu *cpu) { bit(cpu, 1, cpu->registers.d); }
static void bit_1_e(struct Cpu *cpu) { bit(cpu, 1, cpu->registers.e); }
static void bit_1_h(struct Cpu *cpu) { bit(cpu, 1, cpu->registers.h); }
static void bit_1_l(struct Cpu *cpu) { bit(cpu, 1, cpu->registers.l); }
static void bit_1_mem_hl(struct Cpu *cpu) { bit(cpu, 1, read_mem_at_hl(cpu)); }
//
static void bit_2_a(struct Cpu *cpu) { bit(cpu, 2, cpu->registers.a); }
static void bit_2_b(struct Cpu *cpu) { bit(cpu, 2, cpu->registers.b); }
static void bit_2_c(struct Cpu *cpu) { bit(cpu, 2, cpu->registers.c); }
static void bit_2_d(struct Cpu *cpu) { bit(cpu, 2, cpu->registers.d); }
static void bit_2_e(struct Cpu *cpu) { bit(cpu, 2, cpu->registers.e); }
static void bit_2_h(struct Cpu *cpu) { bit(cpu, 2, cpu->registers.h); }
static void bit_2_l(struct Cpu *cpu) { bit(cpu, 2, cpu->registers.l); }
static void bit_2_mem_hl(struct Cpu *cpu) { bit(cpu, 2, read_mem_at_hl(cpu)); }
//
static void bit_3_a(struct Cpu *cpu) { bit(cpu, 3, cpu->registers.a); }
static void bit_3_b(struct Cpu *cpu) { bit(cpu, 3, cpu->registers.b); }
static void bit_3_c(struct Cpu *cpu) { bit(cpu, 3, cpu->registers.c); }
static void bit_3_d(struct Cpu *cpu) { bit(cpu, 3, cpu->registers.d); }
static void bit_3_e(struct Cpu *cpu) { bit(cpu, 3, cpu->registers.e); }
static void bit_3_h(struct Cpu *cpu) { bit(cpu, 3, cpu->registers.h); }
static void bit_3_l(struct Cpu *cpu) { bit(cpu, 3, cpu->registers.l); }
static void bit_3_mem_hl(struct Cpu *cpu) { bit(cpu, 3, read_mem_at_hl(cpu)); }
//
static void bit_4_a(struct Cpu *cpu) { bit(cpu, 4, cpu->registers.a); }
static void bit_4_b(struct Cpu *cpu) { bit(cpu, 4, cpu->registers.b); }
static void bit_4_c(struct Cpu *cpu) { bit(cpu, 4, cpu->registers.c); }
static void bit_4_d(struct Cpu *cpu) { bit(cpu, 4, cpu->registers.d); }
static void bit_4_e(struct Cpu *cpu) { bit(cpu, 4, cpu->registers.e); }
static void bit_4_h(struct Cpu *cpu) { bit(cpu, 4, cpu->registers.h); }
static void bit_4_l(struct Cpu *cpu) { bit(cpu, 4, cpu->registers.l); }
static void bit_4_mem_hl(struct Cpu *cpu) { bit(cpu, 4, read_mem_at_hl(cpu)); }
//
static void bit_5_a(struct Cpu *cpu) { bit(cpu, 5, cpu->registers.a); }
static void bit_5_b(struct Cpu *cpu) { bit(cpu, 5, cpu->registers.b); }
static void bit_5_c(struct Cpu *cpu) { bit(cpu, 5, cpu->registers.c); }
static void bit_5_d(struct Cpu *cpu) { bit(cpu, 5, cpu->registers.d); }
static void bit_5_e(struct Cpu *cpu) { bit(cpu, 5, cpu->registers.e); }
static void bit_5_h(struct Cpu *cpu) { bit(cpu, 5, cpu->registers.h); }
static void bit_5_l(struct Cpu *cpu) { bit(cpu, 5, cpu->registers.l); }
static void bit_5_mem_hl(struct Cpu *cpu) { bit(cpu, 5, read_mem_at_hl(cpu)); }
//
static void bit_6_a(struct Cpu *cpu) { bit(cpu, 6, cpu->registers.a); }
static void bit_6_b(struct Cpu *cpu) { bit(cpu, 6, cpu->registers.b); }
static void bit_6_c(struct Cpu *cpu) { bit(cpu, 6, cpu->registers.c); }
static void bit_6_d(struct Cpu *cpu) { bit(cpu, 6, cpu->registers.d); }
static void bit_6_e(struct Cpu *cpu) { bit(cpu, 6, cpu->registers.e); }
static void bit_6_h(struct Cpu *cpu) { bit(cpu, 6, cpu->registers.h); }
static void bit_6_l(struct Cpu *cpu) { bit(cpu, 6, cpu->registers.l); }
static void bit_6_mem_hl(struct Cpu *cpu) { bit(cpu, 6, read_mem_at_hl(cpu)); }
//
static void bit_7_a(struct Cpu *cpu) { bit(cpu, 7, cpu->registers.a); }
static void bit_7_b(struct Cpu *cpu) { bit(cpu, 7, cpu->registers.b); }
static void bit_7_c(struct Cpu *cpu) { bit(cpu, 7, cpu->registers.c); }
static void bit_7_d(struct Cpu *cpu) { bit(cpu, 7, cpu->registers.d); }
static void bit_7_e(struct Cpu *cpu) { bit(cpu, 7, cpu->registers.e); }
static void bit_7_h(struct Cpu *cpu) { bit(cpu, 7, cpu->registers.h); }
static void bit_7_l(struct Cpu *cpu) { bit(cpu, 7, cpu->registers.l); }
static void bit_7_mem_hl(struct Cpu *cpu) { bit(cpu, 7, read_mem_at_hl(cpu)); }



static uint8_t res(uint8_t bitnum, uint8_t value)
{
    return value & ~(1 << bitnum);
}
static void res_0_a(struct Cpu *cpu) { cpu->registers.a = res(0, cpu->registers.a); }
static void res_0_b(struct Cpu *cpu) { cpu->registers.b = res(0, cpu->registers.b); }
static void res_0_c(struct Cpu *cpu) { cpu->registers.c = res(0, cpu->registers.c); }
static void res_0_d(struct Cpu *cpu) { cpu->registers.d = res(0, cpu->registers.d); }
static void res_0_e(struct Cpu *cpu) { cpu->registers.e = res(0, cpu->registers.e); }
static void res_0_h(struct Cpu *cpu) { cpu->registers.h = res(0, cpu->registers.h); }
static void res_0_l(struct Cpu *cpu) { cpu->registers.l = res(0, cpu->registers.l); }
static void res_0_hl(struct Cpu *cpu) { write_mem_at_hl(cpu, res(0, read_mem_at_hl(cpu))); }
//
static void res_1_a(struct Cpu *cpu) { cpu->registers.a = res(1, cpu->registers.a); }
static void res_1_b(struct Cpu *cpu) { cpu->registers.b = res(1, cpu->registers.b); }
static void res_1_c(struct Cpu *cpu) { cpu->registers.c = res(1, cpu->registers.c); }
static void res_1_d(struct Cpu *cpu) { cpu->registers.d = res(1, cpu->registers.d); }
static void res_1_e(struct Cpu *cpu) { cpu->registers.e = res(1, cpu->registers.e); }
static void res_1_h(struct Cpu *cpu) { cpu->registers.h = res(1, cpu->registers.h); }
static void res_1_l(struct Cpu *cpu) { cpu->registers.l = res(1, cpu->registers.l); }
static void res_1_hl(struct Cpu *cpu) { write_mem_at_hl(cpu, res(1, read_mem_at_hl(cpu))); }
//
static void res_2_a(struct Cpu *cpu) { cpu->registers.a = res(2, cpu->registers.a); }
static void res_2_b(struct Cpu *cpu) { cpu->registers.b = res(2, cpu->registers.b); }
static void res_2_c(struct Cpu *cpu) { cpu->registers.c = res(2, cpu->registers.c); }
static void res_2_d(struct Cpu *cpu) { cpu->registers.d = res(2, cpu->registers.d); }
static void res_2_e(struct Cpu *cpu) { cpu->registers.e = res(2, cpu->registers.e); }
static void res_2_h(struct Cpu *cpu) { cpu->registers.h = res(2, cpu->registers.h); }
static void res_2_l(struct Cpu *cpu) { cpu->registers.l = res(2, cpu->registers.l); }
static void res_2_hl(struct Cpu *cpu) { write_mem_at_hl(cpu, res(2, read_mem_at_hl(cpu))); }
//
static void res_3_a(struct Cpu *cpu) { cpu->registers.a = res(3, cpu->registers.a); }
static void res_3_b(struct Cpu *cpu) { cpu->registers.b = res(3, cpu->registers.b); }
static void res_3_c(struct Cpu *cpu) { cpu->registers.c = res(3, cpu->registers.c); }
static void res_3_d(struct Cpu *cpu) { cpu->registers.d = res(3, cpu->registers.d); }
static void res_3_e(struct Cpu *cpu) { cpu->registers.e = res(3, cpu->registers.e); }
static void res_3_h(struct Cpu *cpu) { cpu->registers.h = res(3, cpu->registers.h); }
static void res_3_l(struct Cpu *cpu) { cpu->registers.l = res(3, cpu->registers.l); }
static void res_3_hl(struct Cpu *cpu) { write_mem_at_hl(cpu, res(3, read_mem_at_hl(cpu))); }
//
static void res_4_a(struct Cpu *cpu) { cpu->registers.a = res(4, cpu->registers.a); }
static void res_4_b(struct Cpu *cpu) { cpu->registers.b = res(4, cpu->registers.b); }
static void res_4_c(struct Cpu *cpu) { cpu->registers.c = res(4, cpu->registers.c); }
static void res_4_d(struct Cpu *cpu) { cpu->registers.d = res(4, cpu->registers.d); }
static void res_4_e(struct Cpu *cpu) { cpu->registers.e = res(4, cpu->registers.e); }
static void res_4_h(struct Cpu *cpu) { cpu->registers.h = res(4, cpu->registers.h); }
static void res_4_l(struct Cpu *cpu) { cpu->registers.l = res(4, cpu->registers.l); }
static void res_4_hl(struct Cpu *cpu) { write_mem_at_hl(cpu, res(4, read_mem_at_hl(cpu))); }
//
static void res_5_a(struct Cpu *cpu) { cpu->registers.a = res(5, cpu->registers.a); }
static void res_5_b(struct Cpu *cpu) { cpu->registers.b = res(5, cpu->registers.b); }
static void res_5_c(struct Cpu *cpu) { cpu->registers.c = res(5, cpu->registers.c); }
static void res_5_d(struct Cpu *cpu) { cpu->registers.d = res(5, cpu->registers.d); }
static void res_5_e(struct Cpu *cpu) { cpu->registers.e = res(5, cpu->registers.e); }
static void res_5_h(struct Cpu *cpu) { cpu->registers.h = res(5, cpu->registers.h); }
static void res_5_l(struct Cpu *cpu) { cpu->registers.l = res(5, cpu->registers.l); }
static void res_5_hl(struct Cpu *cpu) { write_mem_at_hl(cpu, res(5, read_mem_at_hl(cpu))); }
//
static void res_6_a(struct Cpu *cpu) { cpu->registers.a = res(6, cpu->registers.a); }
static void res_6_b(struct Cpu *cpu) { cpu->registers.b = res(6, cpu->registers.b); }
static void res_6_c(struct Cpu *cpu) { cpu->registers.c = res(6, cpu->registers.c); }
static void res_6_d(struct Cpu *cpu) { cpu->registers.d = res(6, cpu->registers.d); }
static void res_6_e(struct Cpu *cpu) { cpu->registers.e = res(6, cpu->registers.e); }
static void res_6_h(struct Cpu *cpu) { cpu->registers.h = res(6, cpu->registers.h); }
static void res_6_l(struct Cpu *cpu) { cpu->registers.l = res(6, cpu->registers.l); }
static void res_6_hl(struct Cpu *cpu) { write_mem_at_hl(cpu, res(6, read_mem_at_hl(cpu))); }
//
static void res_7_a(struct Cpu *cpu) { cpu->registers.a = res(7, cpu->registers.a); }
static void res_7_b(struct Cpu *cpu) { cpu->registers.b = res(7, cpu->registers.b); }
static void res_7_c(struct Cpu *cpu) { cpu->registers.c = res(7, cpu->registers.c); }
static void res_7_d(struct Cpu *cpu) { cpu->registers.d = res(7, cpu->registers.d); }
static void res_7_e(struct Cpu *cpu) { cpu->registers.e = res(7, cpu->registers.e); }
static void res_7_h(struct Cpu *cpu) { cpu->registers.h = res(7, cpu->registers.h); }
static void res_7_l(struct Cpu *cpu) { cpu->registers.l = res(7, cpu->registers.l); }
static void res_7_hl(struct Cpu *cpu) { write_mem_at_hl(cpu, res(7, read_mem_at_hl(cpu))); }


static uint8_t sla(struct Cpu *cpu, uint8_t value)
{
    uint8_t newValue = value << 1;
    cpu->flags.zero = newValue == 0;
    cpu->flags.negative = false;
    cpu->flags.halfCarry = false;
    cpu->flags.carry = (value & 0x80) != 0;
    return newValue;
}
static void sla_a(struct Cpu *cpu) { cpu->registers.a = sla(cpu, cpu->registers.a); }
static void sla_b(struct Cpu *cpu) { cpu->registers.b = sla(cpu, cpu->registers.b); }
static void sla_c(struct Cpu *cpu) { cpu->registers.c = sla(cpu, cpu->registers.c); }
static void sla_d(struct Cpu *cpu) { cpu->registers.d = sla(cpu, cpu->registers.d); }
static void sla_e(struct Cpu *cpu) { cpu->registers.e = sla(cpu, cpu->registers.e); }
static void sla_h(struct Cpu *cpu) { cpu->registers.h = sla(cpu, cpu->registers.h); }
static void sla_l(struct Cpu *cpu) { cpu->registers.l = sla(cpu, cpu->registers.l); }
static void sla_mem_hl(struct Cpu *cpu) { write_mem_at_hl(cpu, sla(cpu, read_mem_at_hl(cpu))); }


static uint8_t srl(struct Cpu *cpu, uint8_t value)
{
    uint8_t newValue = value >> 1;
    cpu->flags.zero = newValue == 0;
    cpu->flags.negative = false;
    cpu->flags.halfCarry = false;
    cpu->flags.carry = (value & 0x01) != 0;
    return newValue;
}
static void srl_a(struct Cpu *cpu) { cpu->registers.a = srl(cpu, cpu->registers.a); }
static void srl_b(struct Cpu *cpu) { cpu->registers.b = srl(cpu, cpu->registers.b); }
static void srl_c(struct Cpu *cpu) { cpu->registers.c = srl(cpu, cpu->registers.c); }
static void srl_d(struct Cpu *cpu) { cpu->registers.d = srl(cpu, cpu->registers.d); }
static void srl_e(struct Cpu *cpu) { cpu->registers.e = srl(cpu, cpu->registers.e); }
static void srl_h(struct Cpu *cpu) { cpu->registers.h = srl(cpu, cpu->registers.h); }
static void srl_l(struct Cpu *cpu) { cpu->registers.l = srl(cpu, cpu->registers.l); }
static void srl_mem_hl(struct Cpu *cpu) { write_mem_at_hl(cpu, srl(cpu, read_mem_at_hl(cpu))); }



static uint8_t rlc(struct Cpu *cpu, uint8_t value)
{
    uint8_t newValue = (value << 1) | ((value & 0x80) ? 1 : 0);
    cpu->flags.zero = newValue == 0;
    cpu->flags.negative = false;
    cpu->flags.halfCarry = false;
    cpu->flags.carry = (value & 0x80) != 0;
    return newValue;
}
static void rlc_a(struct Cpu *cpu) { cpu->registers.a = rlc(cpu, cpu->registers.a); }
static void rlc_b(struct Cpu *cpu) { cpu->registers.b = rlc(cpu, cpu->registers.b); }
static void rlc_c(struct Cpu *cpu) { cpu->registers.c = rlc(cpu, cpu->registers.c); }
static void rlc_d(struct Cpu *cpu) { cpu->registers.d = rlc(cpu, cpu->registers.d); }
static void rlc_e(struct Cpu *cpu) { cpu->registers.e = rlc(cpu, cpu->registers.e); }
static void rlc_h(struct Cpu *cpu) { cpu->registers.h = rlc(cpu, cpu->registers.h); }
static void rlc_l(struct Cpu *cpu) { cpu->registers.l = rlc(cpu, cpu->registers.l); }
static void rlc_mem_hl(struct Cpu *cpu) { write_mem_at_hl(cpu, rlc(cpu, read_mem_at_hl(cpu))); }

// These non-CB versions are exactly like the CB versions except that the
// zero flag is always cleared.
static void rlca(struct Cpu *cpu)
{
    rlc_a(cpu);
    cpu->flags.zero = false;
}







const struct Instruction instructions[256] = {
    // 0x00
    { "NOP",             0, nop },
    { "LD BC, 0x%04x",   2, ld_bc_d16 },
    { "LD (BC), A",      0, ld_mem_bc_a },
    { "INC BC",          0, inc_bc },
    { "INC B",           0, inc_b },
    { "DEC B",           0, dec_b },
    { "LD B, 0x%02x",    1, ld_b_d8 },
    { "RLCA",            0, rlca },
    { "LD (0x%04x), SP", 2, ld_mem_a16_sp },
    { "ADD HL, BC",      0, add_hl_bc },
    { "LD A, (BC)",      0, ld_a_mem_bc },
    { "DEC BC",          0, dec_bc },
    { "INC C",           0, inc_c },
    { "DEC C",           0, dec_c },
    { "LD C, 0x%02x",    1, ld_c_d8 },
    { "RRCA",            0, NULL },

    // 0x10
    { "STOP %d",         1, NULL },
    { "LD DE, 0x%04x",   2, ld_de_d16 },
    { "LD (DE), A",      0, ld_mem_de_a },
    { "INC DE",          0, inc_de },
    { "INC D",           0, inc_d },
    { "DEC D",           0, dec_d },
    { "LD D, 0x%02x",    1, ld_d_d8 },
    { "RLA",             0, NULL },
    { "JR %hhd",         1, jr },
    { "ADD HL, DE",      0, add_hl_de },
    { "LD A, (DE)",      0, ld_a_mem_de },
    { "DEC DE",          0, dec_de },
    { "INC E",           0, inc_e },
    { "DEC E",           0, dec_e },
    { "LD E, 0x%02x",    1, ld_e_d8 },
    { "RRA",             0, NULL },

    // 0x20
    { "JR NZ, %hhd",     1, jr_nz },
    { "LD HL, 0x%04x",   2, ld_hl_d16 },
    { "LD (HL+), A",     0, ld_mem_hlp_a },
    { "INC HL",          0, inc_hl },
    { "INC H",           0, inc_h },
    { "DEC H",           0, dec_h },
    { "LD H, 0x%02x",    1, ld_h_d8 },
    { "DAA",             0, daa },
    { "JR Z, %hhd",      1, jr_z },
    { "ADD HL, HL",      0, add_hl_hl },
    { "LD A, (HL+)",     0, ld_a_mem_hlp },
    { "DEC HL",          0, dec_hl },
    { "INC L",           0, inc_l },
    { "DEC L",           0, dec_l },
    { "LD L, 0x%02x",    1, ld_l_d8 },
    { "CPL",             0, cpl },

    // 0x30
    { "JR NC, %hhd",     1, jr_nc },
    { "LD SP, 0x%04x",   2, ld_sp_d16 },
    { "LD (HL-), A",     0, ld_mem_hlm_a },
    { "INC SP",          0, inc_sp },
    { "INC (HL)",        0, inc_mem_hl },
    { "DEC (HL)",        0, dec_mem_hl },
    { "LD (HL), 0x%02x", 1, ld_mem_hl_d8 },
    { "SCF",             0, NULL },
    { "JR C, %hhd",      1, jr_c },
    { "ADD HL, SP",      0, add_hl_sp },
    { "LD A, (HL-)",     0, ld_a_mem_hlm },
    { "DEC SP",          0, dec_sp },
    { "INC A",           0, inc_a },
    { "DEC A",           0, dec_a },
    { "LD A, 0x%02x",    1, ld_a_d8 },
    { "CCF",             0, NULL },

    // 0x40
    { "LD B, B",         0, ld_b_b },
    { "LD B, C",         0, ld_b_c },
    { "LD B, D",         0, ld_b_d },
    { "LD B, E",         0, ld_b_e },
    { "LD B, H",         0, ld_b_h },
    { "LD B, L",         0, ld_b_l },
    { "LD B, (HL)",      0, ld_b_hl },
    { "LD B, A",         0, ld_b_a },
    { "LD C, B",         0, ld_c_b },
    { "LD C, C",         0, ld_c_c },
    { "LD C, D",         0, ld_c_d },
    { "LD C, E",         0, ld_c_e },
    { "LD C, H",         0, ld_c_h },
    { "LD C, L",         0, ld_c_l },
    { "LD C, (HL)",      0, ld_c_hl },
    { "LD C, A",         0, ld_c_a },

    // 0x50
    { "LD D, B",         0, ld_d_b },
    { "LD D, C",         0, ld_d_c },
    { "LD D, D",         0, ld_d_d },
    { "LD D, E",         0, ld_d_e },
    { "LD D, H",         0, ld_d_h },
    { "LD D, L",         0, ld_d_l },
    { "LD D, (HL)",      0, ld_d_hl },
    { "LD D, A",         0, ld_d_a },
    { "LD E, B",         0, ld_e_b },
    { "LD E, C",         0, ld_e_c },
    { "LD E, D",         0, ld_e_d },
    { "LD E, E",         0, ld_e_e },
    { "LD E, H",         0, ld_e_h },
    { "LD E, L",         0, ld_e_l },
    { "LD E, (HL)",      0, ld_e_hl },
    { "LD E, A",         0, ld_e_a },

    // 0x60
    { "LD H, B",         0, ld_h_b },
    { "LD H, C",         0, ld_h_c },
    { "LD H, D",         0, ld_h_d },
    { "LD H, E",         0, ld_h_e },
    { "LD H, H",         0, ld_h_h },
    { "LD H, L",         0, ld_h_l },
    { "LD H, (HL)",      0, ld_h_hl },
    { "LD H, A",         0, ld_h_a },
    { "LD L, B",         0, ld_l_b },
    { "LD L, C",         0, ld_l_c },
    { "LD L, D",         0, ld_l_d },
    { "LD L, E",         0, ld_l_e },
    { "LD L, H",         0, ld_l_h },
    { "LD L, L",         0, ld_l_l },
    { "LD L, (HL)",      0, ld_l_hl },
    { "LD L, A",         0, ld_l_a },

    // 0x70
    { "LD (HL), B",      0, ld_hl_b },
    { "LD (HL), C",      0, ld_hl_c },
    { "LD (HL), D",      0, ld_hl_d },
    { "LD (HL), E",      0, ld_hl_e },
    { "LD (HL), H",      0, ld_hl_h },
    { "LD (HL), L",      0, ld_hl_l },
    { "HALT",            0, NULL },
    { "LD (HL), A",      0, ld_hl_a },
    { "LD A, B",         0, ld_a_b },
    { "LD A, C",         0, ld_a_c },
    { "LD A, D",         0, ld_a_d },
    { "LD A, E",         0, ld_a_e },
    { "LD A, H",         0, ld_a_h },
    { "LD A, L",         0, ld_a_l },
    { "LD A, (HL)",      0, ld_a_hl },
    { "LD A, A",         0, ld_a_a },

    // 0x80
    { "ADD A, B",        0, add_b },
    { "ADD A, C",        0, add_c },
    { "ADD A, D",        0, add_d },
    { "ADD A, E",        0, add_e },
    { "ADD A, H",        0, add_h },
    { "ADD A, L",        0, add_l },
    { "ADD A, (HL)",     0, add_hl },
    { "ADD A, A",        0, add_a },
    { "ADC A, B",        0, adc_b },
    { "ADC A, C",        0, adc_c },
    { "ADC A, D",        0, adc_d },
    { "ADC A, E",        0, adc_e },
    { "ADC A, H",        0, adc_h },
    { "ADC A, L",        0, adc_l },
    { "ADC A, (HL)",     0, adc_hl },
    { "ADC A, A",        0, adc_a },

    // 0x90
    { "SUB B",           0, sub_b },
    { "SUB C",           0, sub_c },
    { "SUB D",           0, sub_d },
    { "SUB E",           0, sub_e },
    { "SUB H",           0, sub_h },
    { "SUB L",           0, sub_l },
    { "SUB (HL)",        0, sub_hl },
    { "SUB A",           0, sub_a },
    { "SBC A, B",        0, NULL },
    { "SBC A, C",        0, NULL },
    { "SBC A, D",        0, NULL },
    { "SBC A, E",        0, NULL },
    { "SBC A, H",        0, NULL },
    { "SBC A, L",        0, NULL },
    { "SBC A, (HL)",     0, NULL },
    { "SBC A, A",        0, NULL },

    // 0xa0
    { "AND B",           0, and_b },
    { "AND C",           0, and_c },
    { "AND D",           0, and_d },
    { "AND E",           0, and_e },
    { "AND H",           0, and_h },
    { "AND L",           0, and_l },
    { "AND (HL)",        0, and_hl },
    { "AND A",           0, and_a },
    { "XOR B",           0, xor_b },
    { "XOR C",           0, xor_c },
    { "XOR D",           0, xor_d },
    { "XOR E",           0, xor_e },
    { "XOR H",           0, xor_h },
    { "XOR L",           0, xor_l },
    { "XOR (HL)",        0, xor_hl },
    { "XOR A",           0, xor_a },

    // 0xb0
    { "OR B",            0, or_b },
    { "OR C",            0, or_c },
    { "OR D",            0, or_d },
    { "OR E",            0, or_e },
    { "OR H",            0, or_h },
    { "OR L",            0, or_l },
    { "OR (HL)",         0, or_hl },
    { "OR A",            0, or_a },
    { "CP B",            0, cp_b },
    { "CP C",            0, cp_c },
    { "CP D",            0, cp_d },
    { "CP E",            0, cp_e },
    { "CP H",            0, cp_h },
    { "CP L",            0, cp_l },
    { "CP (HL)",         0, cp_mem_hl },
    { "CP A",            0, cp_a },

    // 0xc0
    { "RET NZ",          0, ret_nz },
    { "POP BC",          0, pop_bc },
    { "JP NZ, 0x%04x",   2, jp_nz_a16 },
    { "JP 0x%04x",       2, jp_a16 },
    { "CALL NZ, 0x%04x", 2, NULL },
    { "PUSH BC",         0, push_bc },
    { "ADD A, 0x%02x",   1, add_d8 },
    { "RST 0x00",        0, rst00 },
    { "RET Z",           0, ret_z },
    { "RET",             0, ret },
    { "JP Z, 0x%04x",    2, jp_z_a16 },
    { "<prefix cb>",     0, NULL },
    { "CALL Z, 0x%04x",  2, NULL },
    { "CALL 0x%04x",     2, call_a16 },
    { "ADC A, 0x%02x",   1, adc_d8 },
    { "RST 0x08",        0, rst08 },

    // 0xd0
    { "RET NC",          0, ret_nc },
    { "POP DE",          0, pop_de },
    { "JP NC, 0x%04x",   2, jp_nc_a16 },
    { "<undocumented>",  0, NULL },
    { "CALL NC, 0x%04x", 2, NULL },
    { "PUSH DE",         0, push_de },
    { "SUB 0x%02x",      1, sub_d8 },
    { "RST 0x10",        0, rst10 },
    { "RET C",           0, ret_c },
    { "RETI",            0, reti },
    { "JP C, 0x%04x",    2, jp_c_a16 },
    { "<undocumented>",  0, NULL },
    { "CALL C, 0x%04x",  2, NULL },
    { "<undocumented>",  0, NULL },
    { "SBC A, 0x%02x",   1, NULL },
    { "RST 0x18",        0, rst18 },

    // 0xe0
    { "LDH (0x%02x), A", 1, ldh_a8_a },
    { "POP HL",          0, pop_hl },
    { "LD (C), A",       0, ld_mem_c_a },
    { "<undocumented>",  0, NULL },
    { "<undocumented>",  0, NULL },
    { "PUSH HL",         0, push_hl },
    { "AND 0x%02x",      1, and_d8 },
    { "RST 0x20",        0, rst20 },
    { "ADD SP, %hhd",    1, NULL },
    { "JP (HL)",         0, jp_hl },
    { "LD (0x%04x), A",  2, ld_mem_a16_a },
    { "<undocumented>",  0, NULL },
    { "<undocumented>",  0, NULL },
    { "<undocumented>",  0, NULL },
    { "XOR A, 0x%02x",   1, NULL },
    { "RST 0x28",        0, rst28 },

    // 0xf0
    { "LDH A, (0x%02x)", 1, ldh_a_a8 },
    { "POP AF",          0, pop_af },
    { "LD A, (C)",       0, NULL },
    { "DI",              0, di },
    { "<undocumented>",  0, NULL },
    { "PUSH AF",         0, push_af },
    { "OR 0x%02x",       1, or_d8 },
    { "RST 0x30",        0, rst30 },
    { "LD HL, SP%+d",    1, NULL },
    { "LD SP, HL",       0, ld_sp_hl },
    { "LD A, (0x%04x)",  2, ld_a_mem_a16 },
    { "EI",              0, ei },
    { "<undocumented>",  0, NULL },
    { "<undocumented>",  0, NULL },
    { "CP A, 0x%02x",    1, cp_d8 },
    { "RST 0x38",        0, rst38 },
};













const struct Instruction cbInstructions[256] = {
    // 0x00
    { "RLC B",      0, rlc_b },
    { "RLC C",      0, rlc_c },
    { "RLC D",      0, rlc_d },
    { "RLC E",      0, rlc_e },
    { "RLC H",      0, rlc_h },
    { "RLC L",      0, rlc_l },
    { "RLC (HL)",   0, rlc_mem_hl },
    { "RLC A",      0, rlc_a },
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
    { "SLA B",      0, sla_b },
    { "SLA C",      0, sla_c },
    { "SLA D",      0, sla_d },
    { "SLA E",      0, sla_e },
    { "SLA H",      0, sla_h },
    { "SLA L",      0, sla_l },
    { "SLA (HL)",   0, sla_mem_hl },
    { "SLA A",      0, sla_a },
    { "SRA B",      0, NULL },
    { "SRA C",      0, NULL },
    { "SRA D",      0, NULL },
    { "SRA E",      0, NULL },
    { "SRA H",      0, NULL },
    { "SRA L",      0, NULL },
    { "SRA (HL)",   0, NULL },
    { "SRA A",      0, NULL },

    // 0x30
    { "SWAP B",     0, swap_b },
    { "SWAP C",     0, swap_c },
    { "SWAP D",     0, swap_d },
    { "SWAP E",     0, swap_e },
    { "SWAP H",     0, swap_h },
    { "SWAP L",     0, swap_l },
    { "SWAP (HL)",  0, swap_hl },
    { "SWAP A",     0, swap_a },
    { "SRL B",      0, srl_b },
    { "SRL C",      0, srl_c },
    { "SRL D",      0, srl_d },
    { "SRL E",      0, srl_e },
    { "SRL H",      0, srl_h },
    { "SRL L",      0, srl_l },
    { "SRL (HL)",   0, srl_mem_hl },
    { "SRL A",      0, srl_a },

    // 0x40
    { "BIT 0 B",      0, bit_0_b },
    { "BIT 0 C",      0, bit_0_c },
    { "BIT 0 D",      0, bit_0_d },
    { "BIT 0 E",      0, bit_0_e },
    { "BIT 0 H",      0, bit_0_h },
    { "BIT 0 L",      0, bit_0_l },
    { "BIT 0 (HL)",   0, bit_0_mem_hl },
    { "BIT 0 A",      0, bit_0_a },
    { "BIT 1 B",      0, bit_1_b },
    { "BIT 1 C",      0, bit_1_c },
    { "BIT 1 D",      0, bit_1_d },
    { "BIT 1 E",      0, bit_1_e },
    { "BIT 1 H",      0, bit_1_h },
    { "BIT 1 L",      0, bit_1_l },
    { "BIT 1 (HL)",   0, bit_1_mem_hl },
    { "BIT 1 A",      0, bit_1_a },

    // 0x50
    { "BIT 2 B",      0, bit_2_b },
    { "BIT 2 C",      0, bit_2_c },
    { "BIT 2 D",      0, bit_2_d },
    { "BIT 2 E",      0, bit_2_e },
    { "BIT 2 H",      0, bit_2_h },
    { "BIT 2 L",      0, bit_2_l },
    { "BIT 2 (HL)",   0, bit_2_mem_hl },
    { "BIT 2 A",      0, bit_2_a },
    { "BIT 3 B",      0, bit_3_b },
    { "BIT 3 C",      0, bit_3_c },
    { "BIT 3 D",      0, bit_3_d },
    { "BIT 3 E",      0, bit_3_e },
    { "BIT 3 H",      0, bit_3_h },
    { "BIT 3 L",      0, bit_3_l },
    { "BIT 3 (HL)",   0, bit_3_mem_hl },
    { "BIT 3 A",      0, bit_3_a },

    // 0x60
    { "BIT 4 B",      0, bit_4_b },
    { "BIT 4 C",      0, bit_4_c },
    { "BIT 4 D",      0, bit_4_d },
    { "BIT 4 E",      0, bit_4_e },
    { "BIT 4 H",      0, bit_4_h },
    { "BIT 4 L",      0, bit_4_l },
    { "BIT 4 (HL)",   0, bit_4_mem_hl },
    { "BIT 4 A",      0, bit_4_a },
    { "BIT 5 B",      0, bit_5_b },
    { "BIT 5 C",      0, bit_5_c },
    { "BIT 5 D",      0, bit_5_d },
    { "BIT 5 E",      0, bit_5_e },
    { "BIT 5 H",      0, bit_5_h },
    { "BIT 5 L",      0, bit_5_l },
    { "BIT 5 (HL)",   0, bit_5_mem_hl },
    { "BIT 5 A",      0, bit_5_a },

    // 0x70
    { "BIT 6 B",      0, bit_6_b },
    { "BIT 6 C",      0, bit_6_c },
    { "BIT 6 D",      0, bit_6_d },
    { "BIT 6 E",      0, bit_6_e },
    { "BIT 6 H",      0, bit_6_h },
    { "BIT 6 L",      0, bit_6_l },
    { "BIT 6 (HL)",   0, bit_6_mem_hl },
    { "BIT 6 A",      0, bit_6_a },
    { "BIT 7 B",      0, bit_7_b },
    { "BIT 7 C",      0, bit_7_c },
    { "BIT 7 D",      0, bit_7_d },
    { "BIT 7 E",      0, bit_7_e },
    { "BIT 7 H",      0, bit_7_h },
    { "BIT 7 L",      0, bit_7_l },
    { "BIT 7 (HL)",   0, bit_7_mem_hl },
    { "BIT 7 A",      0, bit_7_a },

    // 0x80
    { "RES 0 B",      0, res_0_b },
    { "RES 0 C",      0, res_0_c },
    { "RES 0 D",      0, res_0_d },
    { "RES 0 E",      0, res_0_e },
    { "RES 0 H",      0, res_0_h },
    { "RES 0 L",      0, res_0_l },
    { "RES 0 (HL)",   0, res_0_hl },
    { "RES 0 A",      0, res_0_a },
    { "RES 1 B",      0, res_1_b },
    { "RES 1 C",      0, res_1_c },
    { "RES 1 D",      0, res_1_d },
    { "RES 1 E",      0, res_1_e },
    { "RES 1 H",      0, res_1_h },
    { "RES 1 L",      0, res_1_l },
    { "RES 1 (HL)",   0, res_1_hl },
    { "RES 1 A",      0, res_1_a },

    // 0x90
    { "RES 2 B",      0, res_2_b },
    { "RES 2 C",      0, res_2_c },
    { "RES 2 D",      0, res_2_d },
    { "RES 2 E",      0, res_2_e },
    { "RES 2 H",      0, res_2_h },
    { "RES 2 L",      0, res_2_l },
    { "RES 2 (HL)",   0, res_2_hl },
    { "RES 2 A",      0, res_2_a },
    { "RES 3 B",      0, res_3_b },
    { "RES 3 C",      0, res_3_c },
    { "RES 3 D",      0, res_3_d },
    { "RES 3 E",      0, res_3_e },
    { "RES 3 H",      0, res_3_h },
    { "RES 3 L",      0, res_3_l },
    { "RES 3 (HL)",   0, res_3_hl },
    { "RES 3 A",      0, res_3_a },

    // 0xa0
    { "RES 4 B",      0, res_4_b },
    { "RES 4 C",      0, res_4_c },
    { "RES 4 D",      0, res_4_d },
    { "RES 4 E",      0, res_4_e },
    { "RES 4 H",      0, res_4_h },
    { "RES 4 L",      0, res_4_l },
    { "RES 4 (HL)",   0, res_4_hl },
    { "RES 4 A",      0, res_4_a },
    { "RES 5 B",      0, res_5_b },
    { "RES 5 C",      0, res_5_c },
    { "RES 5 D",      0, res_5_d },
    { "RES 5 E",      0, res_5_e },
    { "RES 5 H",      0, res_5_h },
    { "RES 5 L",      0, res_5_l },
    { "RES 5 (HL)",   0, res_5_hl },
    { "RES 5 A",      0, res_5_a },

    // 0xb0
    { "RES 6 B",      0, res_6_b },
    { "RES 6 C",      0, res_6_c },
    { "RES 6 D",      0, res_6_d },
    { "RES 6 E",      0, res_6_e },
    { "RES 6 H",      0, res_6_h },
    { "RES 6 L",      0, res_6_l },
    { "RES 6 (HL)",   0, res_6_hl },
    { "RES 6 A",      0, res_6_a },
    { "RES 7 B",      0, res_7_b },
    { "RES 7 C",      0, res_7_c },
    { "RES 7 D",      0, res_7_d },
    { "RES 7 E",      0, res_7_e },
    { "RES 7 H",      0, res_7_h },
    { "RES 7 L",      0, res_7_l },
    { "RES 7 (HL)",   0, res_7_hl },
    { "RES 7 A",      0, res_7_a },

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
