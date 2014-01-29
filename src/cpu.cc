/*
 * Copyright (c) 2014 Kyle Isom <kyle@tyrfingr.is>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <iomanip>
#include <iostream>
#include <cstring>
#include "cpu.h"
#include "ram.h"


// The aaa bits of instructions.
static const uint8_t	aaa = (1 << 7) | (1 << 6) | (1 << 5);

// The bbb bits of instructions.
static const uint8_t	bbb = (1 << 4) | (1 << 3) | (1 << 2);

// The cc bits of instructions.
static const uint8_t	cc = (1 << 1) | (1 << 0);

// C01 address modes
static const uint8_t	C01_MODE_IIZPX = 0;
static const uint8_t	C01_MODE_ZP = 1;
static const uint8_t	C01_MODE_IMM = 2;
static const uint8_t	C01_MODE_ABS = 3;
static const uint8_t	C01_MODE_IIZPY = 4;
static const uint8_t	C01_MODE_ZPX = 5;
static const uint8_t	C01_MODE_ABSY = 6;
static const uint8_t	C01_MODE_ABSX = 7;

// C10 address modes
static const uint8_t	C10_MODE_IMM = 0;
static const uint8_t	C10_MODE_ZP = 1;
static const uint8_t	C10_MODE_ACC = 2;
static const uint8_t	C10_MODE_ABS = 3;
static const uint8_t	C10_MODE_ZPX = 5;
static const uint8_t	C10_MODE_ABSX = 7;

static void
debug(const char *s)
{
	if (DEBUG) {
		std::cerr << "[DEBUG] " << s << "\n";
	}
}


static char *
status_flags(cpu_register8 p)
{
	char *status = new char[9];
	memset(status, 0x30, 8);
	status[8] = 0;

	if (p & FLAG_NEGATIVE)
		status[0] = '1';
	if (p & FLAG_OVERFLOW)
		status[1] = '1';
	if (p & FLAG_EXPANSION)
		status[2] = '1';
	if (p & FLAG_BREAK)
		status[3] = '1';
	if (p & FLAG_DECIMAL)
		status[4] = '1';
	if (p & FLAG_INT_DISABLE)
		status[5] = '1';
	if (p & FLAG_ZERO)
		status[6] = '1';
	if (p & FLAG_CARRY)
		status[7] = '1';
	return status;
}


static uint8_t
overflow(uint8_t a, uint8_t b)
{
	uint8_t bit7 = 1 << 7;
	uint8_t carry = ((a << 1 >> 1) + (b << 1 >> 1)) & bit7;
	uint8_t v;

	//  V = (!M7&!N7&C6) | (M7&N7&!C6) 
	v = ((!(a & bit7)) & (!(b & bit7)) & carry);
	v |= ((a & bit7) & (b & bit7) & (!carry));
	return v;
}


CPU::CPU(size_t memory)
{
	debug("INIT MEMORY");
	this->ram = RAM(memory);
	this->reset_registers();
	ram.reset();
}


CPU::CPU()
{
	debug("default ctor");
	this->reset_registers();
}


void
CPU::reset_registers()
{
	debug("RESET REGISTERS");
	this->a = 0;
	this->x = 0;
	this->y = 0;
	this->p = FLAG_EXPANSION;
	this->s = 0;
	this->pc = 0;
}



void
CPU::dump_registers()
{
	size_t	 size = this->ram.size();
	char	*status = status_flags(this->p);
	std::cerr << "\nREGISTER DUMP\n";
	std::cerr << "\tRAM: " << std::dec << size << " bytes\n";
	std::cerr << "\t  A: " << std::hex << (unsigned int)(this->a) << "\n";
	std::cerr << "\t  X: " << std::hex << (unsigned int)(this->x) << "\n";
	std::cerr << "\t  Y: " << std::hex << (unsigned int)(this->y) << "\n";
	std::cerr << "\t  P: " << std::hex << (unsigned int)(this->p) << "\n";
	std::cerr << "\tFLA: " << "NV-BIDZC\n";
	std::cerr << "\tFLA: " << status << "\n";
	std::cerr << "\t  S: " << std::hex << (unsigned int)(this->s) << "\n";
	std::cerr << "\t PC: " << std::hex << this->pc << "\n";

	delete status;
}


void
CPU::dump_memory()
{
	this->ram.dump();
}


void
CPU::load(const void *src, uint16_t offset, uint16_t len)
{
	this->ram.load(src, offset, len);
}


void
CPU::store(void *dest, uint16_t offset, uint16_t len)
{
	this->ram.store(dest, offset, len);
}


void
CPU::step_pc()
{
	this->pc++;
}


void
CPU::step_pc(uint8_t n)
{
	debug("STEP PC");
	if (n & 0x80)
		this->pc -= uint8_t(~n);
	else
		this->pc += n;
}


void
CPU::start_pc(uint16_t loc)
{
	this->pc = loc;
}


void
CPU::ADC(uint8_t op)
{
	uint8_t	v = 0;
	debug("OP: ADC");

	switch ((op & bbb) >> 2) {
	case C01_MODE_IMM:
		debug("MODE: IMM");
		v = this->read_immed();
	}

	if ((uint8_t)(this->a + v) < (this->a))
		this->p |= FLAG_CARRY;
	if (overflow(this->a, v))
		this->p |= FLAG_OVERFLOW;

	this->a += v;

	if (this->a == 0)
		this->p |= FLAG_ZERO;

	if (this->a & 0x80)
		this->p |= FLAG_NEGATIVE;

	if (!(this->a & 0x80))
		this->p &= ~FLAG_NEGATIVE;
}


void
CPU::AND(uint8_t v)
{
	debug("AND IMM");
	this->a &= v;

	if (this->a == 0)
		this->p |= FLAG_ZERO;
	if (this->a & 0x80)
		this->p |= FLAG_NEGATIVE;
}


void
CPU::AND(uint16_t loc)
{
	debug("AND LOC");
	uint8_t		v = this->ram.peek(loc);
	this->AND(v);
}


void
CPU::CMP(uint8_t v)
{
	debug("CMP IMM");
	this->p &= ~(FLAG_CARRY|FLAG_ZERO|FLAG_NEGATIVE);
	if (this->a < v) {
		debug("LT");
		if ((this->a - v) & 0x80)
			this->p |= FLAG_NEGATIVE;
	} else if (this->a == v) {
		debug("EQ");
		this->p |= (FLAG_CARRY|FLAG_ZERO);
	} else if (this->a > v) {
		debug("GT");
		this->p |= FLAG_CARRY;
		if ((this->a - v) & 0x80)
			this->p |= FLAG_NEGATIVE;
	}
}


void
CPU::CMP(uint16_t loc)
{
	debug("CMP LOC");
	uint8_t		v = this->ram.peek(loc);
	this->CMP(v);
}


void
CPU::CPX(uint8_t v)
{
	debug("CPX IMM");
	this->p &= ~(FLAG_CARRY|FLAG_ZERO|FLAG_NEGATIVE);
	if (this->x < v) {
		debug("LT");
		if ((this->x - v) & 0x80)
			this->p |= FLAG_NEGATIVE;
	} else if (this->x == v) {
		debug("EQ");
		this->p |= (FLAG_CARRY|FLAG_ZERO);
	} else if (this->x > v) {
		debug("GT");
		this->p |= FLAG_CARRY;
		if ((this->x - v) & 0x80)
			this->p |= FLAG_NEGATIVE;
	}
}


void
CPU::CPX(uint16_t loc)
{
	debug("CPX LOC");
	uint8_t		v = this->ram.peek(loc);
	this->CPX(v);
}


void
CPU::DEX()
{
	debug("DEX");
	this->x--;
	if (this->x == 0)
		this->p |= FLAG_ZERO;
	else if (this->x == 0xFF)
		this->p |= FLAG_CARRY;
}

void
CPU::INX()
{
	debug("INX");
	this->x++;
	if (this->x == 0)
		this->p |= (FLAG_ZERO | FLAG_CARRY);
}


void
CPU::LDA(uint8_t op)
{
	// uint16_t	addr;

	debug("OP: LDA");
	switch ((op & bbb) >> 2) {
	case C01_MODE_IMM:
		debug("MODE: IMM");
		this->a = this->read_immed();
		break;
	default:
		debug("INVALID ADDRESSING MODE");
	}
	if (this->a == 0)
		this->p |= FLAG_ZERO;
	if (this->a & 0x80)
		this->p |= FLAG_NEGATIVE;
	else
		this->p &= ~FLAG_NEGATIVE;

}


void
CPU::LDX(uint8_t v)
{
	debug("LDA");
	this->x = v;
	if (v == 0)
		this->p |= FLAG_ZERO;
	if (v & 0x80)
		this->p |= FLAG_NEGATIVE;
	else
		this->p &= ~FLAG_NEGATIVE;
}


void
CPU::STA(uint8_t op)
{
	debug("OP: STA");
	switch ((op & bbb) >> 2) {
	case C01_MODE_ABS:
		debug("MODE: ABS");
		this->ram.poke(this->read_addr(C01_MODE_ABS), this->a);
	}
}


void
CPU::STX(uint16_t loc)
{
	debug("STX");
	this->ram.poke(loc, this->x);
}


void
CPU::TAX()
{
	debug("OP: TAX");
	this->x = this->a;
	if (this->a & 0x80)
		this->p |= FLAG_NEGATIVE;
	if (this->a == 0)
		this->p |= FLAG_ZERO;
}


/*
 * CPU flag set/clear methods.
 */

void
CPU::BRK()
{
	debug("BRK");
	this->p |= FLAG_BREAK;
}


void
CPU::CLC()
{
	debug("CLC");
	this->p &= ~FLAG_CARRY;
}


void
CPU::SEC()
{
	debug("SEC");
	this->p |= FLAG_CARRY;
}


void
CPU::CLD()
{
	debug("CLD");
	this->p &= ~FLAG_DECIMAL;
}


void
CPU::SED()
{
	debug("SED");
	this->p |= FLAG_DECIMAL;
}


void
CPU::CLI()
{
	debug("CLI");
	this->p &= ~FLAG_INT_DISABLE;
}


void
CPU::SEI()
{
	debug("SEI");
	this->p |= FLAG_INT_DISABLE;
}


void
CPU::CLV()
{
	debug("CLV");
	this->p &= ~FLAG_OVERFLOW;
}


/*
 * branching instructions
 */

void
CPU::BNE(uint8_t n)
{
	if (this->p & FLAG_ZERO)
		return;
	this->step_pc(n);
}


void
CPU::BNE(uint16_t loc)
{
	if (this->p & FLAG_ZERO)
		return;
	this->pc = loc;
}


/*
 * Instruction processing (reading, parsing, and handling opcodes).
 */

void
CPU::step()
{
	uint8_t		op;

	debug("STEP");
	op = this->ram.peek(this->pc);
	this->step_pc();

	switch (op & cc) {
	case 0x01:
		this->instrc01(op);
		return;
	case 0x02:
		this->instrc10(op);
		return;
	default:
		if (op == 0xe8) {
			std::cerr << "[DEBUG] OP: INX\n";
			this->INX();
		} else if (op == 0x00) {
			std::cerr << "[DEBUG] OP: BRK\n";
			this->BRK();
		} else {
			std::cerr << "[DEBUG] ILLEGAL INSTRUCTION (cc): "
			    << std::setw(2) << std::hex << std::setfill('0')
			    << (unsigned int)(op&0xff) << std::endl;
		}
	}
}


void
CPU::instrc01(uint8_t op)
{
	switch (op >> 5) {
	case 0x3: // ADC
		this->ADC(op);
		return;
	case 0x4: // STA
		this->STA(op);
		return;
	case 0x5: // LDA
		this->LDA(op);
		return;
	default:
		std::cerr << "[DEBUG] ILLEGAL INSTRUCTION (cc): "
			  << std::setw(2) << std::hex << std::setfill('0')
			  << (unsigned int)(op&0xff) << std::endl;
		std::cerr << "[DEBUG] CC = " << (unsigned int)(op >> 5)
			  << "\n";
	}
}


void
CPU::instrc10(uint8_t op)
{
	// uint16_t	addr;
	// uint8_t		v;

	switch (op >> 5) {
	case 0x05:	// TAX
		this->TAX();
		break;
	default:
		std::cerr << "[DEBUG] ILLEGAL INSTRUCTION (INVALID 10): "
			  << std::setw(2) << std::hex << std::setfill('0')
			  << (unsigned int)(op&0xff) << std::endl;
		break;
	}
}


uint8_t
CPU::read_immed()
{
	uint8_t	v;
	std::cerr << "[DEBUG] PEEK $" << std::hex << std::setfill('0')
		  << std::setw(4) << this->pc;
	v = this->ram.peek(this->pc);
	this->step_pc();
	std::cerr << ": " << std::setw(2) << ((unsigned int)v & 0xff) << "\n";
	return v;
}


uint16_t
CPU::read_addr(uint8_t mode)
{
	uint16_t	addr;

	switch (mode) {
	case C01_MODE_ABS:
		addr = this->read_immed();
		addr += ((uint16_t)this->read_immed() << 8);
		break;
	default:
		addr = 0;
	}

	std::cerr << "[DEBUG] ADDR: $" << std::setw(4) << std::setfill('0')
		  << std::hex << addr << std::endl;
	return addr;
}
