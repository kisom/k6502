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


#ifndef __6502_CPU_H
#define __6502_CPU_H


#define DEBUG	1


#include <cstdlib>

#include "ram.h"


const uint8_t	FLAG_CARRY = 1 << 0;
const uint8_t	FLAG_ZERO = 1 << 1;
const uint8_t	FLAG_INT_DISABLE = 1 << 2;
const uint8_t	FLAG_DECIMAL = 1 << 3;
const uint8_t	FLAG_BREAK = 1 << 4;
const uint8_t	FLAG_EXPANSION = 1 << 5;
const uint8_t	FLAG_OVERFLOW = 1 << 6;
const uint8_t	FLAG_NEGATIVE = 1 << 7;


typedef uint8_t		cpu_register8;
typedef uint16_t	cpu_register16;


class CPU {
	private:
		cpu_register8	a;
		cpu_register8	x;
		cpu_register8	y;
		cpu_register8	p;
		cpu_register8	s;
		cpu_register16	pc;
		RAM		ram;

		void		reset_registers(void);
		void		instrc01(uint8_t);
		void		instrc10(uint8_t);
		uint8_t		read_immed();
		uint16_t	read_addr(uint8_t);
	public:
		CPU();
		CPU(size_t);

		void dump_registers(void);
		void dump_memory(void);
		void step(void);

		// Memory access
		void load(const void *, uint16_t, uint16_t);
		void store(void *, uint16_t, uint16_t);

		// PC instructions
		void step_pc(void);
		void step_pc(uint8_t);
		void start_pc(uint16_t);

		// status register
		void BRK(void);
		void CLC(void);
		void CLD(void);
		void CLI(void);
		void CLV(void);
		void SEC(void);
		void SED(void);
		void SEI(void);

		// Instructions
		void ADC(uint8_t);
		void AND(uint8_t);
		void AND(uint16_t);
		void CMP(uint8_t);
		void CMP(uint16_t);
		void CPX(uint8_t);
		void CPX(uint16_t);
		void DEX(void);
		void INX(void);
		void LDA(uint8_t);
		void LDX(uint8_t);
		void STA(uint8_t);
		void STX(uint16_t);
		void TAX(void);

		// Branching
		void BNE(uint8_t);
		void BNE(uint16_t);
};


#endif
