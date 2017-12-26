SHELL := /bin/bash

ARCH=$(shell if [ $$(uname -m) == "x86_64" ]; then echo 64; else echo 32; fi;)
CC=gcc
CFLAGS=-m$(ARCH) -std=gnu99 -Wall -Werror -pedantic -Wextra -g
LDLIBS=-lm -lncurses

target=$(shell find . -name '*.c' | head -n 1 | cut -d "/" -f2 | cut -d "." -f1)

.PHONY: compile debug disasm disasm-code ndisasm run runtime runv runvv runall clean
.SUFFIXES:
.DEFAULT_GOAL:=compile

%: %.c
	$(CC) $(CFLAGS) $^ $(LDLIBS) -o $@

$(target) compile: $(target).c
	$(CC) $(CFLAGS) $(target).c $(LDLIBS) -o $(target)

$(target)_g debug: $(target).c
	$(CC) $(CFLAGS) -g $(target).c $(LDLIBS) -o $(target)_g

disasm: $(target)
	objdump -s -x -g -d $(target) > $(target).disasm

disasm-code: $(target)_g
	objdump -s -x -g -S -d $(target)_g > $(target).disasm

ndisasm: $(target)
	objcopy -j .text -S -O binary $(target) $(target).bin
	ndisasm -b$(ARCH) $(target) > $(target).ndisasm

run: $(target)
	./$(target)

runtime: $(target)
	time ./$(target)

runv: $(target)_g
	valgrind ./$(target)_g

runvv: $(target)_g
	time valgrind -v --leak-check=full --show-leak-kinds=all --track-origins=yes --malloc-fill=0x55 ./$(target)_g

runall: $(target)_g
	/usr/bin/time -v valgrind -v --leak-check=full --show-leak-kinds=all --track-origins=yes --malloc-fill=0x55 ./$(target)_g

clean:
	rm -f $(target) $(target)_g $(target).disasm $(target).bin $(target).ndisasm
