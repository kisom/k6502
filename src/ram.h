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


#ifndef __6502_RAM_H
#define __6502_RAM_H


// 131072 bytes is 128k of RAM.
const size_t	DEFAULT_MEM = 131072;


class RAM {
	private:
		unsigned char	*ram;
		size_t		 ram_size;
	public:
		RAM();
		RAM(size_t);

		// Control.
		size_t size();
		void reset(void);

		// Debug.
		void dump(void);

		// Memory location access and store.
		void poke(uint16_t, uint8_t);
		uint8_t peek(uint16_t);

		// Memory load and store.
		void load(const void *, uint16_t, uint16_t);
		void store(void *, uint16_t, uint16_t);
};


#endif
