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

// This is a test of instructions from http://skilldrick.github.io/easy6502/index.html.
// As this library isn't an assembler, it won't assemble the files; I'm
// to check some of the expected behaviours from the emulator.

#include <iomanip>
#include <iostream>
using namespace std;

#include "cpu.h"


void	test1(void);
void	test2(void);
void	test3(void);


static void
dump_program(const unsigned char *program, size_t len)
{
	size_t	i = 0;
	int	l = 0;
	for (i = 0; i < len; ++i) {
		if (l == 0)
			std::cerr << std::setw(8) << std::hex << i << "| ";
		std::cerr << std::hex << std::setw(2) << std::setfill('0')
			  << (unsigned short)(program[i] & 0xff);
		std::cerr << " ";
		l++;
		if (l == 8) {
			std::cerr << " ";
		} else if (l == 16) {
			std::cerr << std::endl;
			l = 0;
		}
	}
	std::cerr << std::endl;
}


static void
run(const unsigned char *program, size_t size, size_t steps, bool trace)
{
	CPU	cpu(0x400);
	std::cerr << "\nPROGRAM:\n";
	dump_program(program, size);
	std::cerr << std::endl;

	cpu.load(program, 0x300, size);
	cpu.start_pc(0x300);

	size_t i;
	for (i = 0; i < steps; ++i) {
		cpu.step();
		if (trace) {
			cpu.dump_memory();
			cpu.dump_registers();
		}
	}
	cpu.dump_memory();
	cpu.dump_registers();
}


void
test1()
{
	std::cerr << "\nStarting test 1\n";
	std::cerr << "\t(First compiled program)\n";

	// test1, compiled as opcodes
	unsigned char	program[] = {0xA9, 0x01, 0x8D, 0x01, 0x00};
	run(program, 5, 2, false);
}


void
test2()
{
	std::cerr << "\nStarting test 2\n";
	std::cerr << "\t(First full compiled easy6502 program)\n";

	unsigned char	program[] = {
		0xa9, 0x01, 0x8d, 0x00, 0x02, 0xa9, 0x05, 0x8d,
		0x01, 0x02, 0xa9, 0x08, 0x8d, 0x02, 0x02
	};
	run(program, 15, 6, false);
}


void
test3()
{
	std::cerr << "\nStarting test 3\n";
	std::cerr << "\t(Second full compiled easy6502 program)\n";

	unsigned char	program[] = {
		0xa9, 0xc0, 0xaa, 0xe8, 0x69, 0xc4, 0x00
	};

	run(program, 7, 5, false);
}


int
main(void)
{
	// test1();
	// test2();
	test3();
}
