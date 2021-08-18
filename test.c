#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>

//shellcode from assembled asm/test.asm
const char *code = "\x55\x48\x89\xe5\x51\x52\x41\x50\x41\x51\x41\x59\x41\x58\x5a\x59\x48\x89\xec\x5d\x68\x40\x10\x40\x00\xc3";

void test()
{
    (*(void (*)())code)();
}

int main(int argc, char *argv[])
{
    puts("main");

    return 0;
}
