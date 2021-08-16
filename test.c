#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

void _start();

//int __attribute__((constructor)) test(){
void test(){
    asm("xor %r9, %r9");
    asm("push %rcx");
    asm("push %rdx");
    asm("push %r8");
    asm("push %r9");

    puts("test");

    asm("pop %r9");
    asm("pop %r8");
    asm("pop %rdx");
    asm("pop %rcx");
    _start();
}

/*void _init(void){
    puts("init");
}*/


int main(int argc, char *argv[]){
    puts("main");
    return 0;
}
