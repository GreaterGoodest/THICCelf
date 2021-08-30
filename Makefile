SUBDIRS =  asm src testing
.PHONY: $(SUBDIRS)

all clean: $(SUBDIRS)
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir -f Makefile $@; \
	done