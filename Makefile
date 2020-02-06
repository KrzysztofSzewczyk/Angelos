
# This file could have been written in more
# elegant manner, but I'm fine with it's current
# state.

# Krzysztof Szewczyk, Jul 2019

CC=gcc
CFLAGS=-Ofast -march=native -funroll-loops -fomit-frame-pointer $(COVERAGE) $(OPTIONS) -Wall -Wextra
TARGETS=bfasm bfi bfintd bconv bfstrip

.PHONY: all clean install uninstall test

all: $(TARGETS) bfasm.b bin

install:
	chmod a+x bin/*
	sudo cp -rf bin/* /bin/

uninstall:
	cd /bin && sudo rm -f $(TARGETS) bfpp bfmake data-labels.pl labels.pl derle.pl bfi-rle && cd -

test: test/*.asm
	chmod a+x test.pl $^
	-bfi --help 2> /dev/null
	-bfi --version 2> /dev/null
	-bfi --blah 2> /dev/null
	./test.pl $^

clean:
	rm -rf bin/

bfasm.b: bfasm bfasm.asm
	./bfasm < bfasm.asm > $@

bin: $(TARGETS)
	mkdir -p bin
	cp $(TARGETS) bfpp bfmake labels.pl derle.pl bfi-rle data-labels.pl bin/
	rm -rf $(TARGETS)
	
test-clean:
	rm -f test/*.aout
	rm -f test/*.b
