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
	debug("init memory");
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
	std::cerr << "REGISTER DUMP\n";
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
CPU::step_pc()
{
	this->pc++;
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
CPU::STA(uint16_t loc)
{
	debug("STA");
	this->ram.poke(loc, this->a);
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
