all: packer test

packer: packer.c
	gcc packer.c -o packer -g

test: test.c
	gcc test.c -o test -g

clean:
	-rm -f test packer
