include = -Ipayload

all: payload/payload.o packer

payload/payload.o:
	$(MAKE) -C payload

packer: packer.c payload/payload.o
	gcc $(include) packer.c payload/payload.o -o packer -g 

clean:
	$(MAKE) -C payload -f Makefile clean