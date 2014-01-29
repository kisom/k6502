AM_CXXFLAGS = -Wall -Wextra -pedantic -Wshadow -Wpointer-arith -Wcast-align
AM_CXXFLAGS += -Wwrite-strings -Wmissing-declarations -Wunused-variable
AM_CXXFLAGS += -Winline -Wno-long-long -Werror -std=c++11 -g

lib_LIBRARIES = libk6502.a
bin_PROGRAMS = easy6502
include_HEADERS = cpu.h ram.h

libk6502_a_SOURCES = cpu.cc ram.cc

easy6502_SOURCES = easy6502.cc
easy6502_LDADD = libk6502.a
