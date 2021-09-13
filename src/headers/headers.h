#ifndef HEADERS
#define HEADERS

#include <stdio.h>
#include <elf.h>

#include "../info/info.h"

Elf64_Addr find_executable_ph(FILE *binary, Elf64_Phdr *ph, target_info t_info);
int get_next_ph(FILE *binary, Elf64_Phdr *ph);
int get_ph_info(FILE *binary, target_info *t_info, Elf64_Addr *entrypoint);
int swap_entry_point(FILE *binary, int entry_address);

#endif