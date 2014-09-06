# Makefile for BitmapLibrary.
# Written in 2013




CXXFLAGS	= -g -Wall -O3 -fPIC -std=gnu++11 -mno-red-zone -fno-exceptions -fno-rtti -c

CXXSRC	= $(shell find . -name "*.cpp")
CXXOBJ	:= $(CXXSRC:.cpp=.o)


all: $(SYSROOT)/usr/lib/libbitmap.a
	@:

$(SYSROOT)/usr/lib/libbitmap.a: $(CXXOBJ)
	@echo libbitmap.a
	@$(AR) rcs $(SYSROOT)/usr/lib/libbitmap.a $(CXXOBJ)

%.o: %.cpp
	@$(CXX) $(CXXFLAGS) -o $@ $<









