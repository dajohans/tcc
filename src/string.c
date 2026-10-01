#include <stdio.h>
#include <stdlib.h>

#include "string.h"
#include "error.h"

string alloc_string(int64_t cap) {
	string tmp = (string){ .c_str = NULL, .len = -1, .cap = -1};
	tmp.c_str = malloc(cap * sizeof(*tmp.c_str));
	if(tmp.c_str == NULL) {
		log_error("Failed to allocate string memory");
		return tmp;
	}
	tmp.len = 0;
	tmp.cap = cap;
	return tmp;
}

void free_string(string str) {
	free(str.c_str);
	str.len = -1;
	str.cap = -1;
}

int64_t c_str_len(char* str){
	int64_t i = 0;
	while(str[i] != '\0') {
		i++;
	}
	return i;
}

string concat_string(string str1, string str2) {
	string tmp = alloc_string(str1.cap + str2.cap - 1);
	if(tmp.cap == -1) {
		log_error("Failed to concatenate strings");
		return tmp;
	}
	for(int64_t i = 0; i < str1.len; i++) {
		tmp.c_str[i] = str1.c_str[i];
	}
	for(int64_t i = 0; i < str2.len; i++) {
		tmp.c_str[str1.len + i] = str2.c_str[i];
	}
	tmp.c_str[tmp.len] = '\0';
	return tmp;
}

string set_string(char* str){
	string tmp = alloc_string(c_str_len(str) + 1);
	if(tmp.cap == -1) {
		log_error("Failed to set string content");
		return tmp;
	}
	tmp.len = tmp.cap - 1;
	for(int64_t i = 0; i < tmp.len; i++) {
		tmp.c_str[i] = str[i];
	}
	tmp.c_str[tmp.len] = '\0';
	return tmp;
}

string stringify_c_str(char *str) {
	string tmp;
	tmp.c_str = str;
	tmp.len = c_str_len(str);
	tmp.cap = tmp.len + 1;
	return tmp;
}

