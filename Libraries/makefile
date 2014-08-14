# Makefile to build static libraries.

all: buildlib

buildlib:
	-@find . -mindepth 1 -maxdepth 1 -type d -not -name "output" -not -name "freetype-2.5.3" -exec make -s -C {} \;
