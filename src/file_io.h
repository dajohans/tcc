#ifndef FILE_IO_H
#define FILE_IO_H

#include <stdio.h>

#include "string.h"

int get_file_size(FILE* fp);
string read_file_c_str(const char* file_name);
string read_file(string file_name);

#endif // FILE_IO_H
