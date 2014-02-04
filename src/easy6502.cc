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

/*
 * This is a test of programs from
 * http://skilldrick.github.io/easy6502/index.html.
 *
 * As this library isn't an assembler, it won't assemble the files; I'm
 * to check some of the expected behaviours from the emulator. Some of
 * the examples had to be changed, as this test CPU uses only 1K of RAM,
 * with a starting PC 0f $0300; the easy6502 VM has much more memory and
 * uses a starting PC of $0600.
 */

#include <sys/time.h>
#include <ctime>
#include <iomanip>
#include <iostream>
using namespace std;

#include "cpu.h"


void	test1(void);
void	test2(void);
void	test3(void);
void	test4(void);
void	test5(void);
void	test6(void);
void	test7(void);
void	test8(void);
void	test9(void);
void	test10(void);
void	test11(void);


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
run(const unsigned char *program, size_t size, bool trace)
{
        CPU	cpu(0x400);
        struct timeval tv_start;
        struct timeval tv_stop;

        std::cerr << "\nPROGRAM:\n";
        dump_program(program, size);
        std::cerr << std::endl;

        cpu.load(program, 0x300, size);
        cpu.set_entry(0x300);

        if (gettimeofday(&tv_start, NULL) != 0)
                std::cerr << "failed to get time of day\n";
        cpu.run(trace);
        if (gettimeofday(&tv_stop, NULL) != 0)
                std::cerr << "failed to get time of day\n";

        cpu.dump_memory();
        cpu.dump_registers();

        size_t usec = (tv_stop.tv_sec * 1000000) -
                      (tv_start.tv_sec * 1000000);
        usec += (tv_stop.tv_usec - tv_start.tv_usec);
        std::cerr << "Run time: " << std::dec << usec << "usec\n";

}


void
test1()
{
        std::cerr << "\nStarting test 1\n";
        std::cerr << "\t(First compiled program)\n";

        // test1, compiled as opcodes
        unsigned char	program[] = {0xA9, 0x01, 0x8D, 0x01, 0x00};
        run(program, 5, false);
}


void
test2()
{
        std::cerr << "\nStarting test 2\n";
        std::cerr << "\t(First full compiled easy6502 program)\n";

        unsigned char	program[] = {
                0xa9, 0x01, 0x8d, 0x00, 0x02, 0xa9, 0x05, 0x8d,
                0x01, 0x02, 0xa9, 0x08, 0x8d, 0x02, 0x02, 0x00
        };
        run(program, 15, false);
}


void
test3()
{
        std::cerr << "\nStarting test 3\n";
        std::cerr << "\t(Second full compiled easy6502 program)\n";

        unsigned char	program[] = {
                0xa9, 0xc0, 0xaa, 0xe8, 0x69, 0xc4, 0x00
        };

        run(program, 7, false);
}


void
test4()
{
        std::cerr << "\nStarting test 4\n";
        std::cerr << "\t(Third full compiled easy6502 program)\n";

        unsigned char	program[] = {
                0xa9, 0x80, 0x85, 0x01, 0x65, 0x01
        };

        run(program, 6, false);
}


void
test5()
{
        std::cerr << "\nStarting test 5\n";
        std::cerr << "\t(First branching easy6502 program)\n";

        unsigned char	program[] = {
                0xa2, 0x08, 0xca, 0x8e, 0x00, 0x02, 0xe0, 0x03,
                0xd0, 0xf8, 0x8e, 0x01, 0x02, 0x00
        };
        run(program, 14, false);
}


void
test6()
{
        std::cerr << "\nStarting test 6\n";
        std::cerr << "\t(Indexed indirect addressing)\n";

        unsigned char	program[] = {
                0xa2, 0x01, 0xa9, 0x05, 0x85, 0x01, 0xa9, 0x03,
                0x85, 0x02, 0xa0, 0x0a, 0x8c, 0x05, 0x03, 0xa1,
                0x00
        };
        run(program, 17, false);
}


void
test7()
{
        std::cerr << "\nStarting test 7\n";
        std::cerr << "\t(Indirect indexed addressing)\n";

        unsigned char	program[] = {
                0xa0, 0x01, 0xa9, 0x03, 0x85, 0x01, 0xa9, 0x01,
                0x85, 0x02, 0xa2, 0x0a, 0x8e, 0x04, 0x01, 0xb1,
                0x01, 0x00
        };
        run(program, 18, false);
}


void
test8()
{
        std::cerr << "\nStarting test 8\n";
        std::cerr << "\t(Stack manipulation 1)\n";

        unsigned char	program[] = {
                0xa2, 0x00, 0xa0, 0x00, 0x8a, 0x99, 0x00, 0x02,
                0x48, 0xe8, 0xc8, 0xc0, 0x10, 0xd0, 0xf5, 0x68,
                0x99, 0x00, 0x02, 0xc8, 0xc0, 0x20, 0xd0, 0xf7,
                0x00
        };
        run(program, 25, false);
}


void
test9()
{
        std::cerr << "\nStarting test 9\n";
        std::cerr << "\t(jump)\n";

        unsigned char	program[] = {
                0xa9, 0x03, 0x4c, 0x08, 0x03, 0x00, 0x00, 0x00,
                0x8d, 0x00, 0x02, 0x00
        };
        run(program, 12, false);
}


void
test10()
{
        std::cerr << "\nStarting test 10\n";
        std::cerr << "\t(JSR/RTS)\n";

        unsigned char	program[] = {
                0x20, 0x09, 0x03, 0x20, 0x0c, 0x03, 0x20, 0x12,
                0x03, 0xa2, 0x00, 0x60, 0xe8, 0xe0, 0x05, 0xd0,
                0xfb, 0x60, 0x00, 0x00
        };
        run(program, 20, false);
}


void
test11()
{
        // This test requires more memory and a different PC to
        // avoid having to rewrite large swaths of 6502 for this test.
        std::cerr << "\nStarting test 11\n";
        std::cerr << "\t(Player-less snake)\n";

        unsigned char	program[] = {
                0x20, 0x06, 0x06, 0x20, 0x38, 0x06, 0x20, 0x0d,
                0x06, 0x20, 0x2a, 0x06, 0x60, 0xa9, 0x02, 0x85,
                0x02, 0xa9, 0x04, 0x85, 0x03, 0xa9, 0x11, 0x85,
                0x10, 0xa9, 0x10, 0x85, 0x12, 0xa9, 0x0f, 0x85,
                0x14, 0xa9, 0x04, 0x85, 0x11, 0x85, 0x13, 0x85,
                0x15, 0x60, 0xa5, 0xfe, 0x85, 0x00, 0xa5, 0xfe,
                0x29, 0x03, 0x18, 0x69, 0x02, 0x85, 0x01, 0x60,
                0x20, 0x4d, 0x06, 0x20, 0x8d, 0x06, 0x20, 0xc3,
                0x06, 0x20, 0x19, 0x07, 0x20, 0x20, 0x07, 0x20,
                0x2d, 0x07, 0x4c, 0x38, 0x06, 0xa5, 0xff, 0xc9,
                0x77, 0xf0, 0x0d, 0xc9, 0x64, 0xf0, 0x14, 0xc9,
                0x73, 0xf0, 0x1b, 0xc9, 0x61, 0xf0, 0x22, 0x60,
                0xa9, 0x04, 0x24, 0x02, 0xd0, 0x26, 0xa9, 0x01,
                0x85, 0x02, 0x60, 0xa9, 0x08, 0x24, 0x02, 0xd0,
                0x1b, 0xa9, 0x02, 0x85, 0x02, 0x60, 0xa9, 0x01,
                0x24, 0x02, 0xd0, 0x10, 0xa9, 0x04, 0x85, 0x02,
                0x60, 0xa9, 0x02, 0x24, 0x02, 0xd0, 0x05, 0xa9,
                0x08, 0x85, 0x02, 0x60, 0x60, 0x20, 0x94, 0x06,
                0x20, 0xa8, 0x06, 0x60, 0xa5, 0x00, 0xc5, 0x10,
                0xd0, 0x0d, 0xa5, 0x01, 0xc5, 0x11, 0xd0, 0x07,
                0xe6, 0x03, 0xe6, 0x03, 0x20, 0x2a, 0x06, 0x60,
                0xa2, 0x02, 0xb5, 0x10, 0xc5, 0x10, 0xd0, 0x06,
                0xb5, 0x11, 0xc5, 0x11, 0xf0, 0x09, 0xe8, 0xe8,
                0xe4, 0x03, 0xf0, 0x06, 0x4c, 0xaa, 0x06, 0x4c,
                0x35, 0x07, 0x60, 0xa6, 0x03, 0xca, 0x8a, 0xb5,
                0x10, 0x95, 0x12, 0xca, 0x10, 0xf9, 0xa5, 0x02,
                0x4a, 0xb0, 0x09, 0x4a, 0xb0, 0x19, 0x4a, 0xb0,
                0x1f, 0x4a, 0xb0, 0x2f, 0xa5, 0x10, 0x38, 0xe9,
                0x20, 0x85, 0x10, 0x90, 0x01, 0x60, 0xc6, 0x11,
                0xa9, 0x01, 0xc5, 0x11, 0xf0, 0x28, 0x60, 0xe6,
                0x10, 0xa9, 0x1f, 0x24, 0x10, 0xf0, 0x1f, 0x60,
                0xa5, 0x10, 0x18, 0x69, 0x20, 0x85, 0x10, 0xb0,
                0x01, 0x60, 0xe6, 0x11, 0xa9, 0x06, 0xc5, 0x11,
                0xf0, 0x0c, 0x60, 0xc6, 0x10, 0xa5, 0x10, 0x29,
                0x1f, 0xc9, 0x1f, 0xf0, 0x01, 0x60, 0x4c, 0x35,
                0x07, 0xa0, 0x00, 0xa5, 0xfe, 0x91, 0x00, 0x60,
                0xa2, 0x00, 0xa9, 0x01, 0x81, 0x10, 0xa6, 0x03,
                0xa9, 0x00, 0x81, 0x10, 0x60, 0xa2, 0x00, 0xea,
                0xea, 0xca, 0xd0, 0xfb, 0x60,
        };
        CPU	cpu(0x800);
        std::cerr << "\nPROGRAM:\n";
        dump_program(program, 0x134);
        std::cerr << std::endl;

        cpu.load(program, 0x600, 0x134);
        cpu.set_entry(0x600);

        cpu.run(true);
        cpu.dump_memory();
        cpu.dump_registers();
}


int
main(void)
{
        test1();
        test2();
        test3();
        test4();
        test5();
        test6();
        test7();
        test8();
        test9();
        test10();
        //test11();
}
