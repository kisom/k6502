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
using namespace std;

#include "cpu.h"


static void
test1(void)
{
	// LDA #$01
	// STA $0200
	// LDA #$05
	// STA $0201
	// LDA #$08
	// STA $0202
	CPU	cpu(0x300);

	cpu.LDA(1);
	cpu.STA(0x200);
	cpu.LDA(5);
	cpu.STA(0x201);
	cpu.LDA(8);
	cpu.STA(0x202);
	cpu.dump_memory();
	cpu.dump_registers();
}


static void
test2(void)
{
	// LDA #$c0  ;Load the hex value $c0 into the A register
	// TAX       ;Transfer the value in the A register to X
	// INX       ;Increment the value in the X register
	// ADC #$c4  ;Add the hex value $c4 to the A register
	// BRK       ;Break - we're done
	CPU	cpu(0x300);

	cpu.start_pc(0x600);
	cpu.dump_registers();
	cpu.LDA(0xc0);
	cpu.step_pc(); cpu.step_pc();
	cpu.dump_registers();
	cpu.TAX();
	cpu.step_pc();
	cpu.dump_registers();
	cpu.INX();
	cpu.step_pc();
	cpu.dump_registers();
	cpu.ADC(0xc4);
	cpu.step_pc(); cpu.step_pc();
	cpu.dump_registers();
	cpu.BRK();
	cpu.step_pc();
	cpu.dump_memory();
	cpu.dump_registers();

	// Expected:
	//	A=$84 X=$c1 Y=$00
	//	SP=$ff PC=$0607
	//	NV-BDIZC
	//	10110001
}


int
main(void)
{
	test1();
	test2();
}




