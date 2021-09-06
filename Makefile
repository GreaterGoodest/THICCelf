SUBDIRS =  asm src testing
.PHONY: $(SUBDIRS)

all: $(SUBDIRS)
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir -f Makefile $@; \
	done
	mkdir -p ./bin
	mv ./src/thicc ./bin/thicc
	mv ./testing/test ./bin/test
	mv ./asm/test.shell ./bin/test.shell

clean: 
	- rm ./bin/*
	- $(MAKE) -C src -f Makefile clean
