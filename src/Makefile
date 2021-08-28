all: packer test

packer: packer.c
	gcc packer.c -o packer -g

test: test.c
	gcc test.c -o test -g -no-pie

clean:
	-rm -f test packer
