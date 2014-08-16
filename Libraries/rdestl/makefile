# Makefile for rdestl.
# Written in 2014




CXX		= ../../build/toolchain/bin/x86_64-orionx-g++
AS		= ../../build/toolchain/bin/x86_64-orionx-as
LD		= ../../build/toolchain/bin/x86_64-orionx-ld
AR		= ../../build/toolchain/bin/x86_64-orionx-ar


CXXFLAGS	= -g -Wall -O3 -fPIC -std=gnu++11 -mno-red-zone -fno-exceptions -fno-rtti -c

CXXSRC	= $(shell find . -name "*.cpp")
CXXOBJ	:= $(CXXSRC:.cpp=.o)


all: ../../build/sysroot/usr/lib/librdestl.a
	@:

../../build/sysroot/usr/lib/librdestl.a: $(CXXOBJ)
	@echo librdestl.a
	@$(AR) rcs ../../build/sysroot/usr/lib/librdestl.a $(CXXOBJ)

%.o: %.cpp
	@$(CXX) $(CXXFLAGS) -o $@ $<









