#include <elf.h>
#include <stdio.h>

#include "elf.h"
#include "info.h"

Elf64_Addr swap_entry_point(FILE *binary, int entry_address)
{
	/* Replaces binary entry point with provided address

    */
	int count = 0;
	Elf64_Ehdr header;
	Elf64_Addr old_entry;

	rewind(binary);

	count = fread(&header, sizeof(Elf64_Ehdr), 1, binary);
	if (count <= 0)
	{
		perror("Unable to read binary");
		return -1;
	}

	old_entry = header.e_entry;
	printf("current entry point: 0x%x\n", old_entry);
	printf("changing to : 0x%x\n", entry_address);

	rewind(binary);

	header.e_entry = entry_address;
	count = fwrite(&header, sizeof(Elf64_Ehdr), 1, binary);
	if (count <= 0)
	{
		perror("Unable to write to binary");
		return -1;
	}

	rewind(binary);

	return old_entry;
}

int get_next_ph(FILE *binary, Elf64_Phdr *ph)
{
	/* Gets the next segment from the provided binary and returns it via segment parameter

    */
	int retval = 0;
	int count = 0;
	Elf64_Phdr program_header;

	count = fread(&program_header, sizeof(Elf64_Phdr), 1, binary);
	if (count < 0)
	{
		perror("Unable to read binary");
		return 1;
	}

	*ph = program_header;
	return retval;
}

Elf64_Addr find_executable_ph(FILE *binary, Elf64_Phdr *ph, target_info t_info)
{
	/* Locates the executable segment within the binary and returns it via segment parameter

    */
	int retval = 0;
	int curr_ph = 0;
	Elf64_Addr curr_ph_start = 0; //addr where current ph begins

	fseek(binary, t_info.ph_start, SEEK_CUR);
	while (curr_ph < t_info.ph_num)
	{
		curr_ph_start = ftell(binary);
		retval = get_next_ph(binary, ph);
		if (retval != 0)
		{
			puts("Failed to get next segment");
			return -1;
		}

		if (ph->p_flags & PF_X) // found exe segment ph?
		{
			return curr_ph_start;
		}

		curr_ph++;
	}

	return -1;
}

int get_ph_info(FILE *binary, target_info *t_info, Elf64_Addr *entrypoint)
{
	/* Get start of program headers, and size of program headers

    */
	int count = 0;
	Elf64_Ehdr header;

	count = fread(&header, sizeof(Elf64_Ehdr), 1, binary);
	if (count <= 0)
	{
		perror("Unable to read binary");
		return 1;
	}
	printf("Program Headers start at: %d\n", header.e_phoff);
	printf("Total program header entries: %d\n", header.e_phnum);

	t_info->ph_start = header.e_phoff;
	t_info->ph_num = header.e_phnum;
	*entrypoint = header.e_entry;

	rewind(binary);
}