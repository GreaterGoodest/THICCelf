#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <elf.h>

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

int get_payload(uint8_t **payload, const char *path)
{
    /* Reads payload from disk and converts to binary
    */
    int retval = 0;
    int payload_size = 0;

    FILE *file = fopen(path, "rb");
    if (file == NULL)
    {
        perror("Failed to open payload");
        return 1;
    }
    retval = fseek(file, 0, SEEK_END);
    if (retval < 0)
    {
        perror("Failed to find end of payload file");
        return 1;
    }
    payload_size = ftell(file);
    if (payload_size < 0)
    {
        perror("ftell failure on payload file");
        return 1;
    }

    *payload = calloc(1, payload_size + 1);
    if (*payload == NULL)
    {
        puts("Failed to allocate memory for payload");
        return 1;
    }

    rewind(file);

    /* Convert hex bytes to binary equivalent */
    int byte = 0;
    int counter = 0;
    const char *formatter = "\\x%02x";
    uint8_t *payload_tracker = *payload;
    while (fscanf(file, formatter, &byte) == 1)
    {
        payload_tracker[counter++] = byte;
    }

    *payload = realloc(*payload, strlen((char *)*payload) + 1);

    fclose(file);
    return strlen((char *)*payload);
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

    old_end = exe_ph.p_paddr + exe_ph.p_filesz;

    exe_ph.p_filesz += payload_size + 1;
    exe_ph.p_memsz += payload_size + 1;

    fwrite(&exe_ph, sizeof(exe_ph), 1, binary);
    rewind(binary);

    return old_end;
}

int inject_payload(FILE *binary, Elf64_Addr prev_exe_ph_end, uint8_t *payload)
{
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
    Elf64_Addr exe_ph_start;    //where executable program header begins
    Elf64_Addr prev_exe_ph_end; //where exe ph ends before modification
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

    retval = get_ph_info(binary, &ph_start, &ph_num);
    if (retval > 0)
    {
        puts("Failed to find segment size");
        retval = 1;
        goto cleanup;
    }

    exe_ph_start = find_executable_ph(binary, &exe_ph, ph_start, ph_num); //using ftell, change exe_segment to ph addr
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

    payload_size = get_payload(&payload, "./asm/test.shell");
    if (retval > 0)
    {
        puts("Failed to load payload");
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

    prev_exe_ph_end = expand_execuable_segment(binary, exe_ph_start, exe_ph, payload_size);
    if (prev_exe_ph_end <= 0)
    {
        puts("Failed to expand executable segment");
        goto cleanup;
    }

    printf("Writing payload to old segment end: 0x%x\n", prev_exe_ph_end);

    retval = inject_payload(binary, prev_exe_ph_end, payload);
    if (retval > 0)
    {
        puts("Failed to inject payload");
        goto cleanup;
    }

    /*retval = swap_entry_point(binary, 0x401122);
    if (retval > 0)
    {
        puts("Failed to swap entry point");
        goto cleanup;
    }

    puts("Successfully overwrote entry point");*/

cleanup:
    fclose(binary);
    if (payload)
    {
        free(payload);
    }
    return retval;
}
