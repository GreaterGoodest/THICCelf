include = -Ipayload -Iheaders -Icaving

all: payload/payload.o packer

payload/payload.o:
	$(MAKE) -C payload

headers/headers.o:
	$(MAKE) -C headers

caving/caving.o:
	$(MAKE) -C caving

packer: packer.c payload/payload.o headers/headers.o caving/caving.o
	gcc $(include) packer.c payload/payload.o headers/headers.o caving/caving.o -o packer -g 

clean:
	$(MAKE) -C payload -f Makefile clean
	$(MAKE) -C headers -f Makefile clean
	$(MAKE) -C caving -f Makefile clean