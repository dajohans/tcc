#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "string.h"
#include "file_io.h"

 
int main() {
	string file_name = set_string("test-file");
	if(file_name.len == -1) {
		return 1;
	}
	string file_content = read_file(file_name);
	/* string file_content = read_file_c_str("test-file"); */
	if(file_content.cap == -1) {
		free_string(file_name);
		return 1;
	}
	printf("Hello, world!\nFile content: '%s'\n", file_content.c_str);
	free_string(file_content);
	free_string(file_name);
	return 0;
}
