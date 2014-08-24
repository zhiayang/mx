# Makefile for UnixCompatLibC.
# Written in 2014




AS		= ../../build/toolchain/bin/x86_64-orionx-as
CXX		= ../../build/toolchain/bin/x86_64-orionx-g++
AR		= ../../build/toolchain/bin/x86_64-orionx-ar

CXXFLAGS	= -g -Wall -O3 -fPIC -std=gnu++11 -mno-red-zone -fno-exceptions -fno-rtti -c


all: ../../build/sysroot/usr/lib/libsyscall.a
	@:

../../build/sysroot/usr/lib/libsyscall.a: source/Functions.cpp.o source/helper.s.o
	@echo libsyscall.a
	@$(AR) rcs ../../build/sysroot/usr/lib/libsyscall.a source/Functions.cpp.o source/helper.s.o

source/Functions.cpp.o: source/Functions.cpp
	@$(CXX) $(CXXFLAGS) -o $@ $<

source/helper.s.o: source/helper.s
	@$(AS) $< -o $@



