#ifndef CAVING
#define CAVING

#include <stdio.h>
#include <elf.h>

int calculate_padding(FILE *binary, Elf64_Phdr exe_phdr, int *padding);
int expand_execuable_segment(FILE *binary, Elf64_Addr exe_ph_start, Elf64_Phdr exe_ph, int payload_size);
int expand_fini(FILE *binary, int payload_size);

#endif