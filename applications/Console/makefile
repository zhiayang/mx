# Makefile for Orion-X4 userspace programs
# Written in 2013



CXX		= ../../build/toolchain/bin/x86_64-orionx-g++
AS		= ../../build/toolchain/bin/x86_64-orionx-as
LD		= ../../build/toolchain/bin/x86_64-orionx-ld

CXXFLAGS	= -Wall -Os -std=gnu++11

CXXSRC	= $(shell find . -name "*.cpp")
CXXOPT	= /Volumes/mx/apps/core/console.x


.PHONY: all createdir

all: $(CXXOPT)
	@:

$(CXXOPT): $(CXXSRC) createdir ../../build/sysroot/usr/lib/libc.a
	@$(CXX) $(CXXFLAGS) -o $@ $(CXXSRC)
	@echo "userspace: console"



createdir:
	@mkdir -p $(dir $(CXXOPT))
