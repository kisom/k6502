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


/*
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
*/


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
CPU::ADC(uint8_t v)
{
	debug("ADC IMM");
	if ((uint8_t)(this->a + v) < (this->a))
		this->p |= (FLAG_CARRY|FLAG_OVERFLOW);

	this->a += v;

	if (this->a == 0)
		this->p |= FLAG_ZERO;

	if (this->a & 0x80)
		this->p |= FLAG_NEGATIVE;

	if (!(this->a & 0x80))
		this->p &= ~FLAG_NEGATIVE;
}


void
CPU::ADC(uint16_t loc)
{
	debug("ADC LOC");
	uint8_t		v = this->ram.peek(loc);
	this->ADC(v);
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
CPU::LDA(uint8_t v)
{
	debug("LDA");
	this->a = v;
	if (v == 0)
		this->p |= FLAG_ZERO;
	if (v & 0x80)
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
CPU::STA(uint16_t loc)
{
	debug("STA");
	this->ram.poke(loc, this->a);
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
	debug("TAX");
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

	if (op & 0x01) {
		this->instrc01(op);
	}
}


void
CPU::instrc01(uint8_t op)
{
	uint16_t	addr;
	uint8_t		v;

	switch (op) {
	case 0xA9:	// LDA#
		std::cerr << "[DEBUG] OP: LDA MODE: IMM\n";
		v = this->ram.peek(this->pc);
		this->step_pc();
		std::cerr << "[DEBUG] LDA #$" << std::setw(2) << std::hex
			  << std::setfill('0') << (unsigned int)(v&0xff) << std::endl;
		this->LDA(v);
		break;
	case 0x8D:	// STA ABS
		std::cerr << "[DEBUG] OP: STA MODE: ABS\n";
		v = this->ram.peek(this->pc);
		std::cerr << "[DEBUG] PEEK $" << std::setw(4) << std::hex
			  << std::setfill('0') << this->pc;
		std::cerr << ": " << std::setw(2) << std::hex << std::setfill('0')
			  << (unsigned int)(v & 0xff) << std::endl;
		addr = v;
		this->step_pc();
		v = this->ram.peek(this->pc);
		std::cerr << "[DEBUG] PEEK $" << std::setw(4) << std::hex
			  << std::setfill('0') << this->pc;
		std::cerr << ": " << std::setw(2) << std::hex << std::setfill('0')
			  << (unsigned int)(v & 0xff) << std::endl;
		addr += (v << 8);
		this->step_pc();
		std::cerr << "[DEBUG] STA $" << std::setw(4) << std::hex
			  << std::setfill('0') << addr << std::endl;
		this->STA(addr);
		break;
	}
}
