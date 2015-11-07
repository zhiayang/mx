# Makefile for stuff.
# Written in 2014

CXXFLAGS	= -g -Wall -O2 -fPIC -std=gnu++11 -mno-red-zone -fno-exceptions -fno-rtti -c

CXXSRC	= $(shell find . -name "*.cpp")
CXXOBJ	= $(CXXSRC:.cpp=.o)
CXXDEPS	= $(CXXSRC:.cpp=.cpp.d)

-include $(CXXDEPS)


all: $(SYSROOT)/usr/lib/libkstl.a
	@:

$(SYSROOT)/usr/lib/libkstl.a: $(CXXOBJ)
	@echo libkstl.a
	@$(AR) rcs $(SYSROOT)/usr/lib/libkstl.a $(CXXOBJ)

%.o: %.cpp
	@$(CXX) $(CXXFLAGS) -MMD -MP -o $@ $<






