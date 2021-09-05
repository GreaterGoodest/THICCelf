#include <elf.h>
#include <stdio.h>
#include <string.h>

#include "caving.h"
#include "headers.h"

int calculate_padding(FILE *binary, Elf64_Phdr exe_phdr, int *padding)
{
	int retval = 0;
	Elf64_Phdr next_phdr;

	get_next_ph(binary, &next_phdr);

	printf("Next segment at: 0x%x\n", next_phdr.p_paddr);
	printf("Executable segment size: %d\n", exe_phdr.p_filesz);

	*padding = next_phdr.p_paddr - (exe_phdr.p_paddr + exe_phdr.p_filesz);

	rewind(binary);
	return retval;
}

int expand_execuable_segment(FILE *binary, Elf64_Addr exe_ph_start, Elf64_Phdr exe_ph, int payload_size)
{
	/* Expands executable segment to fit payload

       Returns new segment end address
    */
	Elf64_Addr old_end = 0;

	rewind(binary);
	printf("exe ph start: %d\n", exe_ph_start);
	fseek(binary, exe_ph_start, SEEK_CUR); //go to exe ph start

	old_end = exe_ph.p_offset + exe_ph.p_filesz;

	exe_ph.p_filesz += payload_size;
	exe_ph.p_memsz += payload_size;

	fwrite(&exe_ph, sizeof(exe_ph), 1, binary);
	rewind(binary);

	return old_end;
}

int expand_fini(FILE *binary, int payload_size)
{
	int section_header_start = 17280;
	Elf64_Addr curr_header_start = 0; //where current sh begins
	Elf64_Shdr curr_header;
	Elf64_Shdr next_header;

	memset(&curr_header, 0, sizeof(curr_header));
	rewind(binary);

	fseek(binary, section_header_start, SEEK_CUR);

	for (;;)
	{
		curr_header_start = ftell(binary);
		fread(&next_header, sizeof(Elf64_Shdr), 1, binary);
		printf("section address: 0x%x\n", curr_header.sh_addr);
		if (next_header.sh_flags & SHF_EXECINSTR) //find first exe sh...
		{
			break;
		}
	}

	/*for (;;)
    {
        next_header_start = ftell(binary);
        fread(&next_header, sizeof(Elf64_Shdr), 1, binary);
        printf("section address: 0x%x\n", curr_header.sh_addr);
        if ((next_header.sh_flags & SHF_EXECINSTR) && !exe_found) //find first exe section
        {
            exe_found = 1;
        }
        if (!(next_header.sh_flags & SHF_EXECINSTR) && exe_found) //last exe section (fini)
        {
            break;
        }
        memcpy(&curr_header, &next_header, sizeof(Elf64_Shdr));
        curr_header = next_header;
    }*/

	printf("fini address: 0x%x\n", curr_header.sh_addr);
	curr_header.sh_size += payload_size;

	rewind(binary);
	fseek(binary, curr_header_start, SEEK_CUR);

	fwrite(&curr_header, sizeof(Elf64_Shdr), 1, binary);

	rewind(binary);

	return 0;
}