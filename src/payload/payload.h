#ifndef PAYLOAD
#define PAYLOAD

#include <elf.h>
#include <stdint.h>

int get_payload(uint8_t **payload, const char *path);
int inject_payload(FILE *binary, Elf64_Addr injection_site, uint8_t *payload, int payload_size);
int stamp_entrypoint(uint8_t *payload, Elf64_Addr entrypoint);

#endif