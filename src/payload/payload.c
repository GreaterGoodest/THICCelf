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