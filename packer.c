#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <elf.h>

int get_next_segment(){

    return 0;
}

int find_executable_segment(){

    return 0;
}

int swap_entry_point(FILE *binary, int entry_address){
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

    retval = find_executable_segment();
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
