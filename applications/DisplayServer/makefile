# Makefile for Orion-X4 userspace programs
# Written in 2013


CXXFLAGS	= -Wall -g -Os -std=gnu++11

CXXSRC	= $(shell find . -name "*.cpp")
CXXOPT	= $(MOUNTPATH)/System/Library/LaunchDaemons/displayd.mxa

.PHONY: all

all: $(CXXOPT)
	@:

$(CXXOPT): $(CXXSRC) $(SYSROOT)/usr/lib/libc.a
	@$(CXX) $(CXXFLAGS) -o $@ $(CXXSRC)
	@$(STRIP) $(CXXOPT)
	@echo "userspace: displayserver"
