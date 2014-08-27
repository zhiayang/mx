# Makefile for Orion-X4 userspace programs
# Written in 2013



CXX		= ../../build/toolchain/bin/x86_64-orionx-g++
AS		= ../../build/toolchain/bin/x86_64-orionx-as
LD		= ../../build/toolchain/bin/x86_64-orionx-ld

CXXFLAGS	= -Wall -s -Os -std=gnu++11 -I../../build/sysroot/usr/local/include/freetype2

CXXSRC	= $(shell find . -name "*.cpp")
CXXOPT	= /Volumes/mx/apps/core/console.x


.PHONY: all createdir

all: $(CXXOPT)
	@:

$(CXXOPT): $(CXXSRC) createdir
	@$(CXX) $(CXXFLAGS) -o $@ $(CXXSRC) -lpng -lz
	@echo "userspace: console"

createdir:
	@mkdir -p $(dir $(CXXOPT))
