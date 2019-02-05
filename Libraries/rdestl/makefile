# Makefile for rdestl.
# Written in 2014




CXXFLAGS	= -g -Wall -O2 -fPIC -std=gnu++11 -mno-red-zone -fno-exceptions -fno-rtti -c

CXXSRC	= $(shell find . -name "*.cpp")
CXXOBJ	= $(CXXSRC:.cpp=.o)
CXXDEPS	= $(CXXOBJ:.o=.cpp.d)

-include $(CXXDEPS)


all: $(SYSROOT)/usr/lib/librdestl.a
	@:

$(SYSROOT)/usr/lib/librdestl.a: $(CXXOBJ)
	@echo librdestl.a
	@$(AR) rcs $(SYSROOT)/usr/lib/librdestl.a $(CXXOBJ)

%.o: %.cpp
	@$(CXX) $(CXXFLAGS) -MMD -MP -o $@ $<









