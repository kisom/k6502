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

#include <cstring>
#include <iomanip>
#include <iostream>
#include "ram.h"


RAM::RAM()
{
	ram_size = DEFAULT_MEM;
	ram = new unsigned char[DEFAULT_MEM];
}


RAM::RAM(size_t bytes)
{
	ram_size = bytes;
	ram = new unsigned char[bytes];
}


void
RAM::reset()
{
	if (this->ram == NULL)
		delete this->ram;
	this->ram = new unsigned char[this->ram_size];
	memset(this->ram, 0x0, this->ram_size);
}


size_t
RAM::size()
{
	return this->ram_size;
}


void
RAM::dump()
{
	size_t	i;
	int	l = 0;

	std::cerr << "\nMEMORY DUMP:\n";
	for (i = 0; i < this->ram_size; ++i) {
		if (l == 0)
			std::cerr << std::setw(8) << std::hex << i << "| ";
		std::cerr << std::hex << std::setw(2) << std::setfill('0')
			  << (unsigned short)(this->ram[i] & 0xff);
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


void
RAM::poke(uint16_t loc, uint8_t val)
{
	this->ram[loc] = val;
}


uint8_t
RAM::peek(uint16_t loc)
{
	return this->ram[loc];
}


void
RAM::load(const void *src, uint16_t offset, uint16_t len)
{
	memcpy(this->ram+offset, src, len);
}


void
RAM::store(void *dest, uint16_t offset, uint16_t len)
{
	memcpy(dest, this->ram+offset, len);
}
