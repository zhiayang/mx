# Makefile for Orion-X3/Orion-X4/mx and derivatives
# Written in 2011
# This makefile is licensed under the WTFPL


X_BUILD_FILE	= .build.h

QEMU		= /usr/local/bin/qemu-system-x86_64
BOCHS		= /usr/local/bin/bochs
MKISOFS	= /usr/local/bin/mkisofs

SYSROOT	= ./build/sysroot
TOOLCHAIN	= ./build/toolchain

CC		= clang
CXX		= clang++
AS		= build/toolchain/bin/x86_64-orionx-as
LD		= build/toolchain/bin/x86_64-orionx-ld
OBJCOPY	= build/toolchain/bin/x86_64-orionx-objcopy
READELF	= build/toolchain/bin/x86_64-orionx-readelf

GCCVERSION	= 4.9.1

WARNINGS	= -Wno-padded -Wno-c++98-compat-pedantic -Wno-c++98-compat -Wno-cast-align -Wno-unreachable-code -Wno-gnu -Wno-missing-prototypes -Wno-switch-enum -Wno-packed -Wno-missing-noreturn -Wno-float-equal -Wno-sign-conversion -Wno-old-style-cast

CXXFLAGS	= -m64 -Weverything -msse3 -g -integrated-as -O2 -fPIC -std=gnu++11 -nostdinc -ffreestanding -mno-red-zone -fno-stack-protector -fno-exceptions -fno-rtti  -I./source/Kernel/HeaderFiles -I./Libraries/Iris/HeaderFiles -I./Libraries/ -I$(SYSROOT)/usr/include -I$(SYSROOT)/usr/include/c++ -target x86_64-elf -c

LDFLAGS	= --gc-sections -z max-page-size=0x1000 -T link.ld -L$(SYSROOT)/usr/lib


MEMORY	= 1024


SSRC		= $(shell find source -iname "*.s")
CXXSRC	= $(shell find source -iname "*.cpp")

SOBJ		= $(SSRC:.s=.s.o)
CXXOBJ	= $(CXXSRC:.cpp=.cpp.o)

CXXDEPS	= $(CXXOBJ:.o=.d)

.DEFAULT_GOAL = all
-include $(CXXDEPS)




LIBRARIES         = -lstdc++ -liris_k -lm -lbitmap -lc -lsyscall -lsupc++ -lgcc
OUTPUT            = build/kernel.mxa


.PHONY: builduserspace buildlib mountdisk clean all cleandisk copyheader



all: $(OUTPUT)
	@# unmount??
	@hdiutil detach -quiet /Volumes/mx

	@echo "# Starting QEMU"
	@$(QEMU) -s -vga std -serial file:"build/serialout.log" -no-reboot -m $(MEMORY) -hda build/disk.img -rtc base=utc -net nic,model=rtl8139 -net user -net dump,file=build/netdump.wcap
	-@rm -f build/.dmf

	@# mount the disk again for inspection.
	@hdiutil attach -quiet build/disk.img

build: $(OUTPUT)
	# built

$(OUTPUT):  mountdisk copyheader $(SYSROOT)/usr/lib/%.a $(SOBJ) $(CXXOBJ) builduserspace
	@echo "# Linking object files"
	@$(LD) $(LDFLAGS) -o build/kernel64.elf source/Kernel/Bootstrap/Start.s.o $(shell find source -name "*.o" ! -name "Start.s.o") $(LIBRARIES)

	@echo "# Performing objcopy"
	@$(OBJCOPY) -g -O elf32-i386 build/kernel64.elf build/kernel.mxa
	@cp $(OUTPUT) /Volumes/mx/boot/kernel.mxa


%.s.o: %.s
	@if [ ! -a build.dmf ]; then tools/updatebuild.sh; fi;
	@touch build/.dmf

	@echo $(notdir $<)
	@$(AS) $< -o $@


%.cpp.o: %.cpp
	@if [ ! -a build.dmf ]; then tools/updatebuild.sh; fi;
	@touch build/.dmf

	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) $(WARNINGS) -MMD -MP -o $@ $<

builduserspace:
	@echo "# Building userspace applications"
	@make -C applications/

copyheader:
	@mkdir -p $(SYSROOT)/usr/lib
	@mkdir -p $(SYSROOT)/usr/include/c++
	@rsync -cmar Libraries/libc/include/* $(SYSROOT)/usr/include/
	@rsync -cmar Libraries/libm/include/* $(SYSROOT)/usr/include/
	@rsync -cmar Libraries/Iris/HeaderFiles/* $(SYSROOT)/usr/include/iris/
	@rsync -cmar Libraries/libsyscall/*.h $(SYSROOT)/usr/include/sys/
	@rsync -cmar $(TOOLCHAIN)/x86_64-orionx/include/c++/$(GCCVERSION)/* $(SYSROOT)/usr/include/c++/
	@rsync -cmar $(SYSROOT)/usr/include/c++/x86_64-orionx/bits/* $(SYSROOT)/usr/include/c++/bits/
	@cp $(TOOLCHAIN)/x86_64-orionx/lib/*.a $(SYSROOT)/usr/lib/
	@cp $(TOOLCHAIN)/lib/gcc/x86_64-orionx/4.9.1/libgcc.a $(SYSROOT)/usr/lib/


buildlib: $(SYSROOT)/usr/lib/%.a
	@:

$(SYSROOT)/usr/lib/%.a:
	@echo "# Building Libraries"
	@make -C Libraries/

mountdisk:
	@test -d /Volumes/mx || hdiutil attach -quiet build/disk.img

cleandisk:
	@find /Volumes/mx -name "*.mxa" | xargs rm
	-@rm /Volumes/mx/hello

clean: cleandisk
	@echo "# Cleaning directory tree"
	@find source -name "*.o" | xargs rm
	@find Libraries -name "*.o" | xargs rm
	@find Libraries -name "*.a" | xargs rm
	@find userspace -name "*.o" | xargs rm
	-@rm build/*.mxa
	-@rm $(CXXDEPS)

