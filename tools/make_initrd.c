#define COMMON_H_NO_EXTRA_LIBS
#define COMMON_H_NO_STD_TYPES
#include <fs/initrd.h>

#define _SYS_TYPES_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
	// first one is this exec name
	++argv;
	int i, num_headers = argc - 1;
	initrd_file_header_t headers[64];
	unsigned int offset = sizeof(initrd_file_header_t) * 64 + sizeof(int);

	printf("size of header: %d\n", sizeof(initrd_file_header_t));

	for (i = 0; i < num_headers; i++) {
		printf("writing file %s at 0x%x\n", argv[i], offset);
		strcpy((char*)headers[i].name, argv[i]);
		headers[i].name[strlen(argv[i])] = '\0';
		headers[i].offset = offset;
		FILE* file = fopen(argv[i], "r");
		if (file == 0) {
			printf("Error: file not found: %s\n", argv[i]);
			return 1;
		}
		fseek(file, 0, SEEK_END);
		headers[i].length = ftell(file);
		offset += headers[i].length;
		fclose(file);
		headers[i].magic = HEADER_MAGIC;
	}

	FILE* initrd = fopen("./initrd.img", "w");
	unsigned char* data = (unsigned char*) malloc(offset);
	fwrite(&num_headers, sizeof(int), 1, initrd);
	fwrite(headers, sizeof(initrd_file_header_t), 64, initrd);

	for (i = 0; i < num_headers; i++) {
		unsigned char* buf = (unsigned char*) malloc(headers[i].length);
		FILE* file = fopen(argv[i], "r");
		fread(buf, 1, headers[i].length, file);
		fwrite(buf, 1, headers[i].length, initrd);
		fclose(file);
		free(buf);
	}

	fclose(initrd);
	free(data);

	return 0;
}
