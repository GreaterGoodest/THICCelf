#ifndef PAYLOAD
#define PAYLOAD

#include <elf.h>
#include <stdint.h>

int get_payload(uint8_t **payload, const char *path);
int stamp_entrypoint(uint8_t *payload, Elf64_Addr entrypoint);

#endif