#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <elf.h>

int get_next_segment(FILE *binary, Elf64_Phdr *segment){
    /* Gets the next segment from the provided binary and returns it via segment parameter

    */
    int retval = 0;
    
    return retval;
}


int find_executable_segment(FILE *binary, Elf64_Phdr *segment){
    /* Locates the executable segment within the binary and returns it via segment parameter

    */
    int retval = 0;
    int found_executable = 0;
    
    while (!found_executable){
        retval = get_next_segment(binary, segment);
        if (retval != 0 ){
            puts("Failed to get next segment");
            goto cleanup;
        }
        found_executable = 1;
    }

cleanup:
    return retval;
}


int swap_entry_point(FILE *binary, int entry_address){
    /* Replaces binary entry point with provided address

    */
    int count = 0;
    Elf64_Ehdr header;

    count = fread(&header, sizeof(Elf64_Ehdr), 1, binary);
    if (count <= 0){
        perror("Unable to read binary");
        return 1;
    }
    printf("current entry point: 0x%x\n", header.e_entry);
    printf("changing to : 0x%x\n", entry_address);

    rewind(binary);

    header.e_entry = entry_address;
    count = fwrite(&header, sizeof(Elf64_Ehdr), 1, binary);
    if (count <= 0){
        perror("Unable to write to binary");
        return 1;
    }

    return 0;
}


int main(int argc, char *argv[]){
    int retval = 0;
    FILE *binary = NULL;
    Elf64_Phdr exe_segment;

    memset(&exe_segment, 0, sizeof(exe_segment));

    if(argc != 2){
        puts("Invalid usage");
        return 1;
    }

    binary = fopen(argv[1], "r+");
    if(NULL == binary){
        perror("Unable to open binary");
        retval = 1;
        goto cleanup;
    }

    retval = find_executable_segment(binary, &exe_segment);
    if (retval > 0){
        puts("Failed to find executable segment");
        goto cleanup;
    }

    retval = swap_entry_point(binary, 0x1135);
    if (retval > 0){
        puts("Failed to swap entry point");
        goto cleanup;
    }
        
    puts("Successfully overwrote entry point");

cleanup:
    fclose(binary);
    return retval;
}
