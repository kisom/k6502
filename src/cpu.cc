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
#include "cpu.h"
#include "ram.h"


static void
debug(const char *s)
{
	if (DEBUG) {
		std::cerr << "[DEBUG] " << s << "\n";
	}
}


CPU::CPU(size_t memory)
{
	debug("init memory");
	this->flags = 0;
	this->ram = RAM(memory);
	this->reset_registers();
	ram.reset();
}


CPU::CPU()
{
	debug("default ctor");
	this->flags = 0;
	this->reset_registers();
}


void
CPU::reset_registers()
{
	debug("RESET REGISTERS");
	this->a = 0;
	this->x = 0;
	this->y = 0;
	this->p = 0;
	this->s = 0;
	this->pc = 0;
}



void
CPU::dump_registers()
{
	size_t	size = this->ram.size();
	std::cerr << "REGISTER DUMP\n";
	std::cerr << "\tRAM: " << std::dec << size << " bytes\n";
	std::cerr << "\t  A: " << std::hex << (unsigned int)(this->a) << "\n";
	std::cerr << "\t  X: " << std::hex << (unsigned int)(this->x) << "\n";
	std::cerr << "\t  Y: " << std::hex << (unsigned int)(this->y) << "\n";
	std::cerr << "\t  P: " << std::hex << (unsigned int)(this->p) << "\n";
	std::cerr << "\t  S: " << std::hex << (unsigned int)(this->s) << "\n";
	std::cerr << "\t PC: " << std::hex << this->pc << "\n";
	std::cerr << "\tFLA: " << std::hex << (unsigned int)this->flags << "\n";
}


void
CPU::dump_memory()
{
	this->ram.dump();
}


void
CPU::LDA(uint8_t v)
{
	this->a = v;
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
}


void
CPU::INX()
{
	debug("INX");
	this->x++;
}


void
CPU::ADC(uint8_t v)
{
	debug("ADC");
	this->a += v;
}


void
CPU::SEC()
{
	debug("SEC");
	this->flags |= FLAG_CARRY;
}


void
CPU::CLC()
{
	debug("CLC");
	this->flags &= ~FLAG_CARRY;
}
