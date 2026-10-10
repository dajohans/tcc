#include <ctype.h>
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

string append_string(string str, string end) {
	if(end.len <= 0) {
		return str;
	}
	if(str.len + end.len >= str.cap - 1) {
		string tmp = alloc_string(str.cap + end.len);
		for(int64_t i = 0; i < str.len; i++) {
			tmp.c_str[i] = str.c_str[i];
		}
		tmp.len = str.len;
		tmp.c_str[tmp.len] = '\0';
		free_string(str);
		str = tmp;
	}
	for(int64_t i = 0; i < end.len; i++) {
		str.c_str[str.len + i] = end.c_str[i];
	}
	str.len = str.len + end.len;
	str.c_str[str.len] = '\0';
	return str;
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
	tmp.len = str1.len + str2.len;
	for(int64_t i = 0; i < str1.len; i++) {
		tmp.c_str[i] = str1.c_str[i];
	}
	for(int64_t i = 0; i < str2.len; i++) {
		tmp.c_str[str1.len + i] = str2.c_str[i];
	}
	tmp.c_str[tmp.len] = '\0';
	return tmp;
}

void free_string(string str) {
	free(str.c_str);
	str.len = -1;
	str.cap = -1;
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

bool str_cmp(string str_1, string str_2) {
	if(str_1.len != str_2.len) {
		return false;
	}
	bool are_equal = true;
	for(int64_t i = 0; i < str_1.len; i++) {
		if(str_1.c_str[i] != str_2.c_str[i]) {
			are_equal = false;
			break;
		}
	}
	return are_equal;
}

string_view set_sv(string str, int64_t start_offset, int64_t len) {
	if(str.cap < 0) {
		return (string_view) { .ptr = NULL, .len = -1 };
	}
	return (string_view) { .ptr = str.c_str + start_offset, .len = len };
}

string_view set_sv_ptr(char* str, int64_t len) {
	if(str == NULL) {
		return (string_view) { .ptr = NULL, .len = -1 };
	}
	return (string_view) { .ptr = str, .len = len };
}

string_view sv_trim_left(string_view* sv) {
	if(sv == NULL || sv->ptr == NULL) {
		return (string_view) { .ptr = NULL, .len = -1 };
	}
	string_view result = (string_view) { .ptr = sv->ptr, .len = sv->len };
	while(sv->len > 0 && isspace(sv->ptr[0]) != 0) {
		sv->ptr = sv->ptr + 1;
		sv->len--;
	}
	result.len = result.len - sv->len;
	return result;
}

string_view sv_trim_right(string_view* sv) {
	if(sv == NULL || sv->ptr == NULL) {
		return (string_view) { .ptr = NULL, .len = -1 };
	}
	string_view result = (string_view) { .ptr = sv->ptr, .len = sv->len };
	while(sv->len > 0 && isspace(sv->ptr[sv->len - 1]) != 0) {
		sv->len--;
	}
	result.ptr = result.ptr + sv->len;
	result.len = result.len - sv->len;
	return result;
}

void sv_trim(string_view* sv) {
	sv_trim_left(sv);
	sv_trim_right(sv);
}

string_view sv_split_at_char(string_view *sv, char c) {
	if(sv == NULL || sv->ptr == NULL) {
		return (string_view) { .ptr = NULL, .len = -1 };
	}
	string_view result = (string_view) { .ptr = sv->ptr, .len = sv->len };
	while(sv->len > 0 && sv->ptr[0] != c) {
		sv->ptr = sv->ptr + 1;
		sv->len--;
	}
	result.len = result.len - sv->len;
	return result;
}

string stringify_c_str(char *str) {
	string tmp;
	tmp.c_str = str;
	tmp.len = c_str_len(str);
	tmp.cap = tmp.len + 1;
	return tmp;
}

