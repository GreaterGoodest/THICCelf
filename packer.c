#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <elf.h>

int get_next_segment(FILE *binary, Elf64_Phdr *segment)
{
    /* Gets the next segment from the provided binary and returns it via segment parameter

    */
    int retval = 0;

    return retval;
}

int find_executable_segment(FILE *binary, Elf64_Phdr *segment, int ph_start, int ph_size, int ph_num)
{
    /* Locates the executable segment within the binary and returns it via segment parameter

    */
    int retval = 0;
    int found_executable = 0;

    while (!found_executable)
    {
        retval = get_next_segment(binary, segment);
        if (retval != 0)
        {
            puts("Failed to get next segment");
            goto cleanup;
        }
        found_executable = 1;
    }

cleanup:
    return retval;
}

int get_ph_info(FILE *binary, int *ph_start, int *ph_size, int *ph_num)
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
    printf("Prgram Header size is: %d\n", header.e_phentsize);
    printf("Total program header entries: %d\n", header.e_phnum);

    *ph_start = header.e_phoff;
    *ph_size = header.e_phentsize;
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

    return 0;
}

int main(int argc, char *argv[])
{
    int retval = 0;
    int ph_start = 0; //start address for program headers
    int ph_size = 0;  //program header (ph) size in this binary
    int ph_num = 0;   //program header (ph) count
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
        goto cleanup;
    }

    retval = get_ph_info(binary, &ph_start, &ph_size, &ph_num);
    if (retval > 0)
    {
        puts("Failed to find segment size");
        retval = 1;
        goto cleanup;
    }

    retval = find_executable_segment(binary, &exe_segment, ph_start, ph_size, ph_num);
    if (retval > 0)
    {
        puts("Failed to find executable segment");
        goto cleanup;
    }

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
