# Makefile for stuff.
# Written in 2014

CXXFLAGS	= -g -Wall -O2 -fPIC -std=gnu++11 -mno-red-zone -fno-exceptions -fno-rtti -c

CXXSRC	= $(shell find . -name "*.cpp")
CXXOBJ	= $(CXXSRC:.cpp=.o)
CXXDEPS	= $(CXXOBJ:.o=.d)

-include $(CXXDEPS)


all: $(SYSROOT)/usr/lib/libacess2_stl.a
	@:

$(SYSROOT)/usr/lib/libacess2_stl.a: $(CXXOBJ)
	@echo libacess2_stl.a
	@$(AR) rcs $(SYSROOT)/usr/lib/libacess2_stl.a $(CXXOBJ)

%.o: %.cpp
	@$(CXX) $(CXXFLAGS) -MMD -MP -o $@ $<









