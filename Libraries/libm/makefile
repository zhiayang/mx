# Makefile for MathLibrary.
# Written in 2014




CXX		= ../../build/toolchain/bin/x86_64-orionx-g++
AS		= ../../build/toolchain/bin/x86_64-orionx-as
LD		= ../../build/toolchain/bin/x86_64-orionx-ld
AR		= ../../build/toolchain/bin/x86_64-orionx-ar


CXXFLAGS	= -Wall -O3 -fPIC -std=gnu++11 -mno-red-zone -fno-exceptions -fno-rtti -c

CXXSRC	= $(shell find . -name "*.cpp")
CXXOBJ	= $(CXXSRC:.cpp=.o)


all: ../../build/sysroot/usr/lib/libm.a
	@:

../../build/sysroot/usr/lib/libm.a: $(COBJ) $(CXXOBJ)
	@echo libm.a
	@$(AR) rcs ../../build/sysroot/usr/lib/libm.a $(COBJ) $(CXXOBJ)

%.o: %.cpp
	@$(CXX) $(CXXFLAGS) -o $@ $<








