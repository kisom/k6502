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
static const uint8_t	C10_MODE_ZPY = 8;
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
	return !((a ^ b) & 0x80);
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
	this->s = 0xff;
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
CPU::run(bool trace)
{
	while(this->step()) {
		if (trace) {
			this->dump_memory();
			this->dump_registers();
		}
	}
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
		this->pc -= uint8_t(~n) + 1;
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
		break;
	default:
		v = this->ram.peek(this->read_addr1((op & bbb) >> 2));
		break;
	}

	std::cerr << "[DEBUG] ADC V: " << std::setw(2) << std::hex
		  << std::setfill('0') << ((unsigned int)v&0xff) << "\n";
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
CPU::CPX(uint8_t op)
{
	uint8_t		v;

	debug("OP: CPX");
	this->p &= ~(FLAG_CARRY|FLAG_ZERO|FLAG_NEGATIVE);

	switch ((op & bbb) >> 2) {
	case C10_MODE_IMM:
		v = this->read_immed();
		break;
	default:
		v = this->ram.peek(this->read_addr0((op & bbb) >> 2));
	}

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
CPU::CPY(uint8_t op)
{
	uint8_t		v;

	debug("OP: CPY");
	this->p &= ~(FLAG_CARRY|FLAG_ZERO|FLAG_NEGATIVE);

	switch ((op & bbb) >> 2) {
	case C10_MODE_IMM:
		v = this->read_immed();
		break;
	default:
		v = this->ram.peek(this->read_addr0((op & bbb) >> 2));
	}

	if (this->y < v) {
		debug("LT");
		if ((this->y - v) & 0x80)
			this->p |= FLAG_NEGATIVE;
	} else if (this->y == v) {
		debug("EQ");
		this->p |= (FLAG_CARRY|FLAG_ZERO);
	} else if (this->y > v) {
		debug("GT");
		this->p |= FLAG_CARRY;
		if ((this->y - v) & 0x80)
			this->p |= FLAG_NEGATIVE;
	}
}


void
CPU::DEX()
{
	debug("OP: DEX");
	this->x--;
	if (this->x == 0)
		this->p |= FLAG_ZERO;
	else if (this->x == 0xFF)
		this->p |= FLAG_CARRY;
}

void
CPU::INX()
{
	debug("OP: INX");
	this->x++;
	if (this->x == 0)
		this->p |= (FLAG_ZERO | FLAG_CARRY);
}


void
CPU::INY()
{
	debug("OP: INY");
	this->y++;
	if (this->y == 0)
		this->p |= (FLAG_ZERO | FLAG_CARRY);
}


void
CPU::LDA(uint8_t op)
{
	debug("OP: LDA");
	switch ((op & bbb) >> 2) {
	case C01_MODE_IMM:
		debug("MODE: IMM");
		this->a = this->read_immed();
		break;
	default:
		this->a = this->ram.peek(this->read_addr1((op & bbb) >> 2));
	}
	if (this->a == 0)
		this->p |= FLAG_ZERO;
	if (this->a & 0x80)
		this->p |= FLAG_NEGATIVE;
	else
		this->p &= ~FLAG_NEGATIVE;

}


void
CPU::LDX(uint8_t op)
{
	debug("OP: LDX");
	switch ((op & bbb) >> 2) {
	case C10_MODE_IMM:
		debug("MODE: IMM");
		this->x = this->read_immed();
		break;
	default:
		debug("INVALID ADDRESSING MODE");
	}
	if (this->x == 0)
		this->p |= FLAG_ZERO;
	if (this->x & 0x80)
		this->p |= FLAG_NEGATIVE;
	else
		this->p &= ~FLAG_NEGATIVE;
}


void
CPU::LDY(uint8_t op)
{
	debug("OP: LDY");
	switch ((op & bbb) >> 2) {
	case C10_MODE_IMM:
		debug("MODE: IMM");
		this->y = this->read_immed();
		break;
	default:
		debug("INVALID ADDRESSING MODE");
	}
	if (this->y == 0)
		this->p |= FLAG_ZERO;
	if (this->y & 0x80)
		this->p |= FLAG_NEGATIVE;
	else
		this->p &= ~FLAG_NEGATIVE;
}


void
CPU::STA(uint8_t op)
{
	debug("OP: STA");
	switch ((op & bbb) >> 2) {
	case C01_MODE_IMM:
		debug("MODE: IMM");
		this->ram.poke((uint16_t)this->read_immed() & 0xff, this->a);
		break;
	default:
		this->ram.poke(this->read_addr1((op & bbb) >> 2), this->a);
		return;
	}
}


void
CPU::STX(uint8_t op)
{
	debug("OP: STX");
	switch ((op & bbb) >> 2) {
	default:
		this->ram.poke(this->read_addr2((op & bbb) >> 2), this->x);
	}
}


void
CPU::STY(uint8_t op)
{
	debug("OP: STY");
	switch ((op & bbb) >> 2) {
	default:
		this->ram.poke(this->read_addr0((op & bbb) >> 2), this->y);
	}
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


void
CPU::TXA()
{
	debug("OP: TXA");
	this->a = this->x;
	if (this->x & 0x80)
		this->p |= FLAG_NEGATIVE;
	if (this->x == 0)
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
CPU::BPL(uint8_t n)
{
	debug("OP: BPL");
	if (this->p & FLAG_NEGATIVE)
		return;
	debug("BRANCH");
	this->step_pc(n);
}


void
CPU::BMI(uint8_t n)
{
	debug("OP: BMI");
	if (!(this->p & FLAG_NEGATIVE))
		return;
	debug("BRANCH");
	this->step_pc(n);
}


void
CPU::BVC(uint8_t n)
{
	debug("OP: BVC");
	if (this->p & FLAG_OVERFLOW)
		return;
	debug("BRANCH");
	this->step_pc(n);
}


void
CPU::BVS(uint8_t n)
{
	debug("OP: BVS");
	if (!(this->p & FLAG_OVERFLOW))
		return;
	debug("BRANCH");
	this->step_pc(n);
}


void
CPU::BCC(uint8_t n)
{
	debug("OP: BCC");
	if (this->p & FLAG_CARRY)
		return;
	debug("BRANCH");
	this->step_pc(n);
}


void
CPU::BCS(uint8_t n)
{
	debug("OP: BCS");
	if (!(this->p & FLAG_CARRY))
		return;
	debug("BRANCH");
	this->step_pc(n);
}


void
CPU::BNE(uint8_t n)
{
	debug("OP: BNE");
	if (this->p & FLAG_ZERO)
		return;
	debug("BRANCH");
	this->step_pc(n);
}


void
CPU::BEQ(uint8_t n)
{
	debug("OP: BEQ");
	if (!(this->p & FLAG_ZERO))
		return;
	debug("BRANCH");
	this->step_pc(n);
}


void
CPU::JMP()
{
	debug("OP: JMP");
	uint16_t	addr = this->read_addr1(C01_MODE_ABS);
	std::cerr << "[DEBUG] JMP ADDR: " << std::setw(4)
		  << std::hex << std::setfill('0')
		  << addr << std::endl;
	this->pc = addr;
}


static uint16_t
stack_addr(uint8_t sp)
{
	return (1 << 8) + sp;
}


void
CPU::JSR()
{
	debug("OP: JSR");
	uint16_t	jaddr = this->read_addr1(C01_MODE_ABS);
	uint16_t	addr = this->pc-1;

	this->ram.poke(stack_addr(this->s--), (uint8_t)(addr >> 8));
	this->ram.poke(stack_addr(this->s--), (uint8_t)(addr << 8 >> 8));
	this->pc = jaddr;
}


void
CPU::RTS()
{
	debug("OP: RTS");
	uint16_t	addr;
	addr = this->ram.peek(stack_addr(++this->s));
	addr += (this->ram.peek(stack_addr(++this->s)) << 8);
	this->pc = addr+1;
}


/*
 * Stack instructions.
 */


void
CPU::PHA()
{
	debug("OP: PHA");
	this->ram.poke((0x01 << 8) + this->s, this->a);
	this->s--;
}


void
CPU::PLA()
{
	debug("OP: PLA");
	this->s++;
	this->a = this->ram.peek((0x01 << 8) + this->s);
}


/*
 * Instruction processing (reading, parsing, and handling opcodes).
 */

bool
CPU::step()
{
	uint8_t		op;

	debug("STEP");
	op = this->ram.peek(this->pc);
	this->step_pc();

	// Scan single-byte opcodes first
	switch (op) {
	case 0x00: // BRK
		this->BRK();
		return false;
	case 0x10: // BPL
		this->BPL(this->read_immed());
		return true;
	case 0x20: // JSR
		this->JSR();
		return true;
	case 0x30: // BMI
		this->BMI(this->read_immed());
		return true;
	case 0x48: // PHA
		this->PHA();
		return true;
	case 0x4C: // JMP
		this->JMP();
		return true;
	case 0x50: // BVC
		this->BVC(this->read_immed());
		return true;
	case 0x60: // RTS
		this->RTS();
		return true;
	case 0x68: // PLA
		this->PLA();
		return true;
	case 0x70: // BVS
		this->BVS(this->read_immed());
		return true;
	case 0x8a: // TXA
		this->TXA();
		return true;
	case 0x90: // BCC
		this->BCC(this->read_immed());
		return true;
	case 0xB0: // BCC
		this->BCS(this->read_immed());
		return true;
	case 0xC8: // INY
		this->INY();
		return true;
	case 0xD0: // BNE
		this->BNE(this->read_immed());
		return true;
	case 0xE8: // INX
		this->INX();
		return true;
	}

	switch (op & cc) {
	case 0x00:
		this->instrc00(op);
		return true;
	case 0x01:
		this->instrc01(op);
		return true;
	case 0x02:
		this->instrc10(op);
		return true;
	default:
		std::cerr << "[DEBUG] ILLEGAL INSTRUCTION (cc): "
			  << std::setw(2) << std::hex << std::setfill('0')
			  << (unsigned int)(op&0xff) << std::endl;
		this->dump_registers();
	}
	return false;
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
		std::cerr << "[DEBUG] ILLEGAL INSTRUCTION (01): "
			  << std::setw(2) << std::hex << std::setfill('0')
			  << (unsigned int)(op&0xff) << std::endl;
		std::cerr << "[DEBUG] OP = " << (unsigned int)op << "\n";
	}
}


void
CPU::instrc10(uint8_t op)
{
	switch (op >> 5) {
	case 0x04:
		this->STX(op);
		break;
	case 0x05:	// LDX / TAX
		if (((op & bbb) >> 2) == C10_MODE_ACC)
			this->TAX();
		else
			this->LDX(op);
		break;
	case 0x06:
		if (0xCA == op)
			this->DEX();
		break;
	default:
		std::cerr << "[DEBUG] ILLEGAL INSTRUCTION (INVALID 10): "
			  << std::setw(2) << std::hex << std::setfill('0')
			  << (unsigned int)(op>>5)
			  << " " << (unsigned int)op << std::endl;
		break;
	}
}


void
CPU::instrc00(uint8_t op)
{
	switch (op >> 5) {
	case 0x04:
		this->STY(op);
		break;
	case 0x05:
		this->LDY(op);
		break;
	case 0x06:
		this->CPY(op);
		break;
	case 0x07:
		this->CPX(op);
		break;
	default:
		this->dump_registers();
		std::cerr << "[DEBUG] ILLEGAL INSTRUCTION (INVALID 00): "
			  << std::setw(2) << std::hex
			  << std::setfill('0')
			  << (unsigned int)(op>>5)
			  << " " << (unsigned int)op << std::endl;
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
CPU::read_addr1(uint8_t mode)
{
	uint16_t	addr;

	switch (mode) {
	case C01_MODE_IIZPX:
		addr = this->read_immed();
		addr += this->x;
		addr = this->ram.peek(addr) + (this->ram.peek(addr+1)<<8);
		break;
	case C01_MODE_ZP:
		addr = this->read_immed();
		break;
	case C01_MODE_ABS:
		addr = this->read_immed();
		addr += ((uint16_t)this->read_immed() << 8);
		break;
	case C01_MODE_IIZPY:
		addr = this->read_immed();
		addr = this->ram.peek(addr) + (this->ram.peek(addr+1)<<8);
		addr += this->y;
		break;
	case C01_MODE_ZPX:
		addr = (uint8_t)(this->read_immed() + this->x);
		break;
	case C01_MODE_ABSY:
		addr = this->read_immed();
		addr += ((uint16_t)this->read_immed() << 8);
		addr += this->y;
		break;
	case C01_MODE_ABSX:
		addr = this->read_immed();
		addr += ((uint16_t)this->read_immed() << 8);
		addr += this->x;
		break;
	default:
		debug("INVALID ADDRESSING MODE");
		std::cerr << "[DEBUG] MODE: " << std::setw(1)<< std::hex
			  << mode << std::endl;
		addr = 0;
	}

	std::cerr << "[DEBUG] ADDR: $" << std::setw(4) << std::setfill('0')
		  << std::hex << addr << std::endl;
	return addr;
}


uint16_t
CPU::read_addr2(uint8_t mode)
{
	uint16_t	addr;

	switch (mode) {
	case C10_MODE_ZP:
		addr = this->read_immed();
		break;
	case C10_MODE_ABS:
		addr = this->read_immed();
		addr += ((uint16_t)this->read_immed() << 8);
		break;
	case C10_MODE_ZPX:
		addr = (uint8_t)(this->read_immed() + this->x);
		break;
	case C10_MODE_ZPY:
		addr = (uint8_t)(this->read_immed() + this->y);
		break;
	case C10_MODE_ABSX:
		addr = this->read_immed();
		addr += ((uint16_t)this->read_immed() << 8);
		addr += this->x;
		break;
	default:
		debug("INVALID ADDRESSING MODE");
		addr = 0;
	}

	std::cerr << "[DEBUG] ADDR: $" << std::setw(4) << std::setfill('0')
		  << std::hex << addr << std::endl;
	return addr;
}


uint16_t
CPU::read_addr0(uint8_t mode)
{
	uint16_t	addr;

	switch (mode) {
	case C10_MODE_ZP:
		addr = this->read_immed();
		break;
	case C10_MODE_ABS:
		addr = this->read_immed();
		addr += ((uint16_t)this->read_immed() << 8);
		break;
	default:
		debug("INVALID ADDRESSING MODE");
		addr = 0;
	}

	std::cerr << "[DEBUG] ADDR: $" << std::setw(4) << std::setfill('0')
		  << std::hex << addr << std::endl;
	return addr;
}


uint8_t
CPU::DMA(uint16_t loc)
{
	return this->ram.peek(loc);
}


void
CPU::DMA(uint16_t loc, uint8_t val)
{
	this->ram.poke(loc, val);
}
