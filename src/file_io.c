#include <stdlib.h>

#include "file_io.h"
#include "error.h"

int get_file_size(FILE* fp) {
	int prev = ftell(fp);
	fseek(fp, 0L, SEEK_END);
	int file_size = ftell(fp);
	fseek(fp, prev, SEEK_SET);
	return file_size;
}

string read_file_c_str(const char* file_name) {
	FILE* fp = fopen(file_name, "r");
	if(fp == NULL) {
		log_error("Failed to open file '%s'", file_name);
		return (string){ .c_str = NULL, .len = -1, .cap = -1 };
	}
	int file_size = get_file_size(fp);
	char* file_content = malloc((file_size + 1) * sizeof(*file_content));
	if(file_content == NULL) {
		log_error("Failed to allocate memory for file '%s'", file_name);
		fclose(fp);
		return (string){ .c_str = NULL, .len = -1, .cap = -1 };
	}
	int ret = fread(file_content, file_size, 1, fp);
	fclose(fp);
	if(ret != 1) {
		log_error("Failed to read file '%s'", file_name);
		free(file_content);
		return (string){ .c_str = NULL, .len = -1, .cap = -1 };
	}
	file_content[file_size] = '\0';
	return stringify_c_str(file_content);
}

string read_file(string file_name) {
	return read_file_c_str(file_name.c_str);
}

