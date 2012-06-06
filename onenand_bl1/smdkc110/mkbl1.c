/*
 * Copyright (c) 2009 Samsung Electronics Co., Ltd.
 *              http://www.samsung.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
/*
 *	1. Compute checksum for S5PC110 iROM  code
 *	2. Pad 0xFFFFFFFF
 *
 *	djpark (2009.08.10)
 */

#include <stdio.h>

//#define DEBUG_MSG

#define DEFAULT_IN_FILE		"BL1.bin"
#define DEFAULT_OUT_FILE	"BL1.bin.padding"


#if defined(S5PC110_EVT1)
#define BL1_LENGTH			(8*1024)
#define BL1_PAD_LENGTH		BL1_LENGTH
#define BL1_PAD2_LENGTH		BL1_LENGTH
#elif defined(S5PC110_EVT0)
#define BL1_LENGTH			(8*1024)		// Page Size: 4KB
#if 1
#define BL1_PAD_LENGTH		(14*1024)		// Page Size: 4KB
#define BL1_PAD2_LENGTH		(16*1024)
#else
#define BL1_PAD_LENGTH		BL1_LENGTH 		// Page Size: 2KB
#define BL1_PAD2_LENGTH		BL1_LENGTH
#endif
#else
#error Application Processor Type is not defined: S5PC110_EVT0 or S5PC110_EVT1
#endif


int get_info(char* file, unsigned int* size, unsigned int* cs)
{
	FILE *fp_read = NULL;
	int ret = -1;
	unsigned int data = 0;
	unsigned int length = 0;
	unsigned int checksum = 0;
	int i;

	fp_read = fopen(file, "rb");
	if (fp_read == NULL)
	{
		printf("File open error! - %s\n", file);
		goto err;
	}

	while (ret = fread(&data, sizeof(unsigned int), 1, fp_read))
	{
		length += 4;

		checksum += ((data >> 0) & 0xff);
		checksum += ((data >> 8) & 0xff);
		checksum += ((data >> 16) & 0xff);
		checksum += ((data >> 24) & 0xff);
	}

#ifdef DEBUG_MSG
	printf("Input File Length: %d Bytes\n", length);
#endif
#if defined(S5PC110_EVT0)
	// 2048 bytes are the maximum file size of the first bootloader.
	if (length > 2048)
	{
		printf("Error: Length of %s SHOULD not be larger than 2048 bytes.\n", file);
		goto err;
	}
#else
	if (length > BL1_LENGTH)
	{
		printf("Error: Length of %s SHOULD not be larger than %d bytes.\n", file, BL1_LENGTH);
		goto err;
	}
#endif

#ifdef DEBUG_MSG
	printf("Checksum: 0x%08X\n", checksum);
#endif

	*size = length;
	*cs = checksum;

	ret = 0;
err:
	if (fp_read != NULL)	fclose(fp_read);

	return ret;
}

int make_image(char* input_file, char* output_file)
{
	FILE *fp_read = NULL;
	FILE *fp_write = NULL;
	int ret = 0;
	unsigned int data = 0;
	unsigned int length = 0;
	unsigned int checksum = 0;
	int i;

	if (get_info(input_file, &length, &checksum))
	{
		printf("Error: Unable to get file informations!\n");
		goto err;
	}

	fp_read = fopen(input_file, "rb");
	if (fp_read == NULL)
	{
		printf("File open error! - %s\n", input_file);
		goto err;
	}

	fp_write = fopen(output_file, "wb");
	if (fp_write == NULL)
	{
		printf("File open error! - %s\n", output_file);
		goto err;
	}

#if defined(S5PC110_EVT1)
	data = 0;
	length = BL1_LENGTH;
	fwrite(&length, sizeof(unsigned int), 1, fp_write);
	fwrite(&data, sizeof(unsigned int), 1, fp_write);
	fwrite(&checksum, sizeof(unsigned int), 1, fp_write);
	fwrite(&data, sizeof(unsigned int), 1, fp_write);

	length = 16;
#else
	length = 0;
#endif

	while (ret = fread(&data, sizeof(unsigned int), 1, fp_read))
	{
		length += 4;
		fwrite(&data, sizeof(unsigned int), 1, fp_write);
	}

	data = 0;
	for (; length < (BL1_PAD_LENGTH-4); length += 4)
	{
		fwrite(&data, sizeof(unsigned int), 1, fp_write);
	}

#if defined(S5PC110_EVT0)
	data = checksum;
	fwrite(&data, sizeof(unsigned int), 1, fp_write);
	length += 4;
#endif

	data = 0;
	for (; length < (BL1_PAD2_LENGTH); length += 4)
	{
		fwrite(&data, sizeof(unsigned int), 1, fp_write);
	}

err:
	if (fp_read != NULL)	fclose(fp_read);
	if (fp_write != NULL)	fclose(fp_write);

	return 0;
}

int main(int argc, char* argv[])
{
	if (argc == 1)
	{
		printf("Default input/output files are used!\n");
		return make_image(DEFAULT_IN_FILE, DEFAULT_OUT_FILE);
	}
	else if (argc == 3)
	{
		return make_image(argv[1], argv[2]);
	}
	else
	{
		printf("Error: Unsupported input parameter!\n");
		printf("usage: %s [<input_file>] [<output_file>]\n", argv[0]);
		return 1;
	}
}
