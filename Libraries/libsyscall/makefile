# Makefile for UnixCompatLibC.
# Written in 2014




CXX		= ../../build/toolchain/bin/x86_64-orionx-g++
AR		= ../../build/toolchain/bin/x86_64-orionx-ar

CXXFLAGS	= -g -Wall -O3 -fPIC -std=gnu++11 -mno-red-zone -fno-exceptions -fno-rtti -c


CXXSRC	= $(shell find source -name "*.cpp")
CXXOBJ	= $(CXXSRC:.cpp=.cpp.o)


all: ../../build/sysroot/usr/lib/libsyscall.a
	@:

../../build/sysroot/usr/lib/libsyscall.a: $(CXXOBJ)
	@echo libsyscall.a
	@$(AR) rcs ../../build/sysroot/usr/lib/libsyscall.a $(CXXOBJ)

%.cpp.o: %.cpp
	@$(CXX) $(CXXFLAGS) -o $@ $<






