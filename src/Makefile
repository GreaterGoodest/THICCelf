include = -Ipayload -Iheaders -Icaving -Iinfo

all: payload/payload.o headers/headers.o caving/caving.o thicc

payload/payload.o:
	$(MAKE) -C payload

headers/headers.o:
	$(MAKE) -C headers

caving/caving.o:
	$(MAKE) -C caving

thicc: thicc.c payload/payload.o headers/headers.o caving/caving.o
	gcc $(include) thicc.c payload/payload.o headers/headers.o caving/caving.o -o thicc -g 

clean:
	$(MAKE) -C payload -f Makefile clean
	$(MAKE) -C headers -f Makefile clean
	$(MAKE) -C caving -f Makefile clean