include = -Ipayload -Iheaders

all: payload/payload.o packer

payload/payload.o:
	$(MAKE) -C payload

headers/headers.o:
	$(MAKE) -C headers

packer: packer.c payload/payload.o headers/headers.o
	gcc $(include) packer.c payload/payload.o headers/headers.o -o packer -g 

clean:
	$(MAKE) -C payload -f Makefile clean
	$(MAKE) -C headers -f Makefile clean