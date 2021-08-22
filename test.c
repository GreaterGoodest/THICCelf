#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>

//shellcode from assembled asm/test.asm
const char *code = "\x55\x48\x89\xe5\x51\x52\x41\x50\x41\x51\x41\x59\x41\x58\x5a\x59\x48\x89\xec\x5d\x68\x40\x10\x40\x00\xc3";

void test()
{
    /*Modify entry point to point here, main will be reached*/
    (*(void (*)())code)();
}

int main(int argc, char *argv[])
{
    puts("main");

    //This is how we will self-modify... but in asm
    int page_size = getpagesize();
    void *test_addr = (void *)test;
    test_addr -= (unsigned long)test_addr % page_size;
    if (mprotect(test_addr, page_size, PROT_READ | PROT_WRITE | PROT_EXEC) == -1)
    {
        perror("mprotect failed");
    }

    return 0;
}
