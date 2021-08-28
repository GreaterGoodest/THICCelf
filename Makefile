.PHONY: asm src

all: asm src

asm: asm
	$(MAKE) -C $@

src: src
	$(MAKE) -C $@

clean:
	$(MAKE) -C src -f Makefile clean
	$(MAKE) -C asm -f Makefile clean