# Makefile for UnixCompatLibC.
# Written in 2014




CXXFLAGS	= -g -Wall -O3 -fPIC -std=gnu++11 -mno-red-zone -fno-exceptions -fno-rtti -c


all: $(SYSROOT)/usr/lib/libsyscall.a
	@:

$(SYSROOT)/usr/lib/libsyscall.a: source/Functions.cpp.o source/helper.s.o
	@echo libsyscall.a
	@$(AR) rcs $(SYSROOT)/usr/lib/libsyscall.a source/Functions.cpp.o source/helper.s.o

source/Functions.cpp.o: source/Functions.cpp
	@$(CXX) $(CXXFLAGS) -o $@ $<

source/helper.s.o: source/helper.s
	@$(AS) $< -o $@



