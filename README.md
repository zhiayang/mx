# [mx] #

The fifth rewrite of what never was, OrionOS.



## Current Features: ##

- Pretty bare now, features will get ported over from Orion-X4 as I get to it.
- Long mode (x86-64)
- BGA Support, works in QEMU, VirtualBox and Bochs.		-> note that you can't actually run the OS in bochs.
- Working IPC system									-> rewrite in progress
- Basic networking support (DHCP, DNS, IPv4, TCP/UDP)	-> porting still in progress.
- Somewhat complete libc written


## TODO List ##
#### High Priority ####
- execve() and family
- Stabilty fixes here and there -- it's gotten much better now
- HPET Support

#### Low Priority ####
- APIC Stuff, including IOAPIC/LAPIC
- Startup an AP CPU for fun?
- Put back the network code from Orion-X4 (and modify to use VFS)
- Fix the StandardIO implementation in Iris -- it's fugly (and broken)
- Finish pthreads beyond basic stuff




## Trying out [mx] ##
#### Required Software ####
- QEMU
- C++ compiler (that targets your own platform)
- Clang is optional but *highly recommended*.
- Sudo access if you're on Linux, not necessary on OS X. Windows not supported (ATM, or ever)

#### Building [mx]'s toolchain ####
- Run the ./bootstrap script (you may have to chmod +x it first. Rest assured it's not malicious :P)
- Everything will be done automatically -- you will need to enter some commands (guided) into QEMU to install GRUB.
- It will take a long time -- the script will first clone patched versions of GNU Binutils and GCC.
- It will then proceed to build these, along with GNU's libstdc++ as well as [mx]'s homegrown libc (the former taking a much longer time)


#### Building the kernel ####
- Once the bootstrap script is done, you should be able to run 'make' in the root directory and have it run.
- If it does not, report an issue on Github.

- Note: if you are not using Clang++ to compile the kernel, you can use the toolchain you just built (found in build/toolchain)
- You will have to modify the CXXFLAGS variable in the makefile to fit GCC instead of Clang. One day I'll patch clang to target this.
- One day.
