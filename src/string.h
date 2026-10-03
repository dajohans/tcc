#ifndef STRING_H
#define STRING_H

#include <stdint.h>

#define C_STR_LEN(str) ((sizeof(str) / sizeof(str[0])) - 1)

typedef struct string {
	char* c_str;
	int64_t len;
	int64_t cap;
} string;

string alloc_string(int64_t cap);
void free_string(string str);
int64_t c_str_len(char* str);
string concat_string(string str1, string str2);
string set_string(char* str);
string stringify_c_str(char *str);
bool str_cmp(string str_1, string str_2);

#endif // STRING_H
