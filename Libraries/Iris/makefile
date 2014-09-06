# Makefile for Orion-X4 userspace library, iris.
# Written in 2013




CXXFLAGS	= -g -Wall -O2 -fPIC -std=gnu++11 -mno-red-zone -fno-exceptions -fno-rtti -I../../source/Kernel/HeaderFiles -c
CXXSRC	= $(shell find . -name "*.cpp")
CXXHDR	= $(shell find . -name "*.hpp")
CXXOBJ	:= $(CXXSRC:.cpp=.o)


.PHONY: all

all: $(SYSROOT)/usr/lib/libiris.a
	@:

$(SYSROOT)/usr/lib/libiris.a: $(CXXOBJ)
	@$(AR) rcs $(SYSROOT)/usr/lib/libiris.a $(CXXOBJ)
	@echo libiris.a

%.o: %.cpp
	@$(CXX) $(CXXFLAGS) -o $@ $<









