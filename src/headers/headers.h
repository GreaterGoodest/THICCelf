#ifndef HEADERS
#define HEADERS

#include <stdio.h>
#include <elf.h>

int find_executable_ph(FILE *binary, Elf64_Phdr *ph, int ph_start, int ph_num);
int get_next_ph(FILE *binary, Elf64_Phdr *ph);
int get_ph_info(FILE *binary, int *ph_start, int *ph_num, Elf64_Addr *entrypoint);

#endif