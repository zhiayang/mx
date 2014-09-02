# Makefile for Orion-X4 userspace library, iris.
# Written in 2013



# todo: change back to gcc when 4.9.2 is released
CXX		= ../../build/toolchain/bin/x86_64-orionx-g++
# CXX		= clang++
AS		= ../../build/toolchain/bin/x86_64-orionx-as
LD		= ../../build/toolchain/bin/x86_64-orionx-ld
AR		= ../../build/toolchain/bin/x86_64-orionx-ar

CXXFLAGS	= -g -Wall -O2 -fPIC -std=gnu++11 -mno-red-zone -fno-exceptions -fno-rtti -I../../source/Kernel/HeaderFiles -c
# CXXFLAGS	= -m64 -integrated-as -g -Wall -O2 -fPIC -std=gnu++11 -mno-red-zone -fno-exceptions -fno-rtti --sysroot=../../build/sysroot -I../../source/Kernel/HeaderFiles -target x86_64-elf -c





CXXSRC	= $(shell find . -name "*.cpp" -maxdepth 1)
CXXSRCK	= $(shell find LibKernel -name "*.cpp")
SYSROOT	= ../../build/sysroot

CXXHDR	= $(shell find . -name "*.hpp")

CXXOBJ	:= $(CXXSRC:.cpp=.o)
CXXOBJK	:= $(CXXSRCK:.cpp=.o)
CXXOBJU	:= $(CXXSRCU:.cpp=.o)


.PHONY: all

all: $(SYSROOT)/usr/lib/libiris.a $(SYSROOT)/usr/lib/libiris_k.a
	@:

$(SYSROOT)/usr/lib/libiris.a: $(CXXOBJ) $(CXXOBJU) $(CXXHDR)
	@$(AR) rcs $(SYSROOT)/usr/lib/libiris.a $(CXXOBJ) $(CXXOBJU)
	@echo libiris.a

$(SYSROOT)/usr/lib/libiris_k.a: $(CXXOBJ) $(CXXOBJK) $(CXXHDR)
	@$(AR) rcs $(SYSROOT)/usr/lib/libiris_k.a $(CXXOBJ) $(CXXOBJK)
	@echo libiris_k.a

%.o: %.cpp
	@$(CXX) $(CXXFLAGS) -o $@ $<









