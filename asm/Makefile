.PHONY: test.shell

all: test.o test.shell

test.o: test.asm
	nasm -f elf64 $< -o $@

test.shell: test.o
	objdump -d test.o | grep '[0-9a-f]:' | grep -v 'file' | cut -f2 -d: | cut -f1-6 -d' ' | tr -s ' ' | tr '\t' ' ' | sed 's/ $$//g' | sed 's/ /\\x/g' | paste -d '' -s > test.shell

clean:
	-rm -f *.o *.shell