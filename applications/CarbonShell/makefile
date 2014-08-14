# Makefile for Orion-X4 userspace programs
# Written in 2013


CXX		= ../../build/toolchain/bin/x86_64-orionx-g++
AS		= ../../build/toolchain/bin/x86_64-orionx-as

CXXFLAGS	= -Wall -O3 -fPIC -mcmodel=large -std=gnu++11 -mno-red-zone -fno-exceptions -fno-rtti -I../../Libraries/Iris/HeaderFiles -I../../source/kernel/HeaderFiles

CXXSRC	= $(shell find . -name "*.cpp")
CXXOBJ	= $(CXXSRC:.cpp=.o)
CXXOPT	= /Volumes/MX/System/Library/CoreServices/CarbonShell.oex

.PHONY: all

all: $(CXXOPT)
	@:

$(CXXOPT): $(CXXSRC)
	@$(CXX) $(CXXFLAGS) -o $@ $(CXXSRC) -liris -lc -lsyscall
	@echo "userspace: CarbonShell.oex"



