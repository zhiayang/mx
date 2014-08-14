# Makefile for UnixCompatLibC.
# Written in 2014




CC		= ../../build/toolchain/bin/x86_64-orionx-gcc
CXX		= ../../build/toolchain/bin/x86_64-orionx-g++
AS		= ../../build/toolchain/bin/x86_64-orionx-as
STRIP		= ../../build/toolchain/bin/x86_64-orionx-strip
AR		= ../../build/toolchain/bin/x86_64-orionx-ar
SYSROOT	= ../../build/sysroot


CFLAGS	= -Wall -O3 -fPIC -std=gnu11 -mno-red-zone -fno-exceptions -c
CXXFLAGS	= -Wall -O3 -fPIC -std=gnu++11 -mno-red-zone -fno-exceptions -fno-rtti -c




CSRC		= $(shell find . -name "*.c")
CXXSRC	= $(shell find . -name "*.cpp")
SSRC		= $(shell find . -name "*.s")

COBJ		= $(CSRC:.c=.c.o)
CXXOBJ	= $(CXXSRC:.cpp=.cpp.o)
SOBJ		= $(SSRC:.s=.o)


all: $(SYSROOT)/usr/lib/libc.a
	@:

$(SYSROOT)/usr/lib/libc.a: $(COBJ) $(CXXOBJ) $(SOBJ)
	@echo libc.a
	@cp $(SYSROOT)/usr/lib/libc.a $(SYSROOT)/usr/lib/libg.a
	@$(AR) rcs $(SYSROOT)/usr/lib/libg.a $(COBJ) $(CXXOBJ) $(SOBJ)
	@cp source/crt0.o $(SYSROOT)/usr/lib/
	@cp source/crti.o $(SYSROOT)/usr/lib/
	@cp source/crtn.o $(SYSROOT)/usr/lib/

%.c.o: %.c
	@$(CC) $(CFLAGS) -o $@ $<

%.cpp.o: %.cpp
	@$(CXX) $(CXXFLAGS) -o $@ $<

%.o: %.s
	@$(AS) $< -o $@







