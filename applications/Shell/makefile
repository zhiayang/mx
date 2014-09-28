# Makefile for mx userspace programs
# Written in 2014

# Specifically for the built-in shell

CXXFLAGS	= -Wall -g -Os -std=gnu++11

CXXSRC	= $(shell find . -name "*.cpp")
CXXOPT	= $(MOUNTPATH)/System/Library/LaunchDaemons/shell.mxa

.PHONY: all

all: $(CXXOPT)
	@:

$(CXXOPT): $(CXXSRC) $(SYSROOT)/usr/lib/libc.a
	@$(CXX) $(CXXFLAGS) -o $@ $(CXXSRC)
	@$(STRIP) --strip-debug $(CXXOPT)
	@echo "userspace: displayserver"
