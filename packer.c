#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <elf.h>

int get_next_ph(FILE *binary, Elf64_Phdr *segment)
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

    *segment = program_header;
    return retval;
}

int find_executable_segment(FILE *binary, Elf64_Phdr *segment, int ph_start, int ph_num)
{
    /* Locates the executable segment within the binary and returns it via segment parameter

    */
    int retval = 0;
    int curr_ph = 0;

    fseek(binary, ph_start, SEEK_CUR);
    while (curr_ph < ph_num)
    {
        retval = get_next_ph(binary, segment);
        if (retval != 0)
        {
            puts("Failed to get next segment");
            return retval;
        }

        if (segment->p_flags & PF_X)
        {
            break;
        }

        curr_ph++;
    }

    return retval;
}

int get_ph_info(FILE *binary, int *ph_start, int *ph_num)
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

    rewind(binary);
}

int swap_entry_point(FILE *binary, int entry_address)
{
    /* Replaces binary entry point with provided address

    */
    int count = 0;
    Elf64_Ehdr header;

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

int main(int argc, char *argv[])
{
    int retval = 0;
    int ph_start = 0; //start address for program headers
    int ph_num = 0;   //program header (ph) count
    int padding = 0;
    FILE *binary = NULL;
    Elf64_Phdr exe_segment;

    memset(&exe_segment, 0, sizeof(exe_segment));

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

    retval = get_ph_info(binary, &ph_start, &ph_num);
    if (retval > 0)
    {
        puts("Failed to find segment size");
        retval = 1;
        goto cleanup;
    }

    retval = find_executable_segment(binary, &exe_segment, ph_start, ph_num);
    if (retval > 0)
    {
        puts("Failed to find executable segment");
        goto cleanup;
    }

    printf("Executable segment at: 0x%x\n", exe_segment.p_paddr);

    retval = calculate_padding(binary, exe_segment, &padding);
    if (retval > 0)
    {
        puts("Failed to calculate available padding");
        goto cleanup;
    }

    printf("Padding available: %d bytes\n", padding);

    retval = swap_entry_point(binary, 0x401122);
    if (retval > 0)
    {
        puts("Failed to swap entry point");
        goto cleanup;
    }

    puts("Successfully overwrote entry point");

cleanup:
    fclose(binary);
    return retval;
}
