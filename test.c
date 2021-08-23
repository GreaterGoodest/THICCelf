#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>

void test()
{
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
