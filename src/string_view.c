#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "string_view.h"
#include "error.h"

string alloc_string(int64_t cap) {
	if(cap <= 0) {
		return (string) { .c_str = NULL, .len = -1, .cap = -1 };
	}
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
	if(str.len + end.len > str.cap - 1) {
		string tmp = alloc_string(str.cap + end.len);
		if(tmp.cap < 0) {
			log_error("Failed to append string");
			return (string) { .c_str = NULL, .len = -1, .cap = -1 };
		}
		for(int64_t i = 0; i < str.len; i++) {
			tmp.c_str[i] = str.c_str[i];
		}
		tmp.len = str.len;
		tmp.c_str[tmp.len] = '\0';
		free_string(&str);
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
	if(str == NULL) {
		return 0;
	}
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

bool free_string(string* str) {
	if(str == NULL) {
		log_error("Unable to free string pointer to NULL");
		return false;
	}
	if(str->c_str == NULL) {
		log_error("Attempted double free of string");
		return false;
	}
	free(str->c_str);
	str->c_str = NULL;
	str->len = -1;
	str->cap = -1;
	return true;
}

string make_string(char* str){
	string tmp = alloc_string(c_str_len(str) + 1);
	if(tmp.cap == -1) {
		log_error("Failed to make string");
		return tmp;
	}
	tmp.len = tmp.cap - 1;
	for(int64_t i = 0; i < tmp.len; i++) {
		tmp.c_str[i] = str[i];
	}
	tmp.c_str[tmp.len] = '\0';
	return tmp;
}

void set_string(string* str, char* new) {
	if(new == NULL) {
		return;
	}
	if(c_str_len(new) > str->cap - 1){
		string tmp = alloc_string(c_str_len(new) + 1);
		if(tmp.cap == -1) {
			log_error("Failed to set string content");
			return;
		}
		free_string(str);
		str->c_str = tmp.c_str;
		str->len = tmp.len;
		str->cap = tmp.cap;
	}
	str->len = c_str_len(new);
	for(int64_t i = 0; i < str->len; i++) {
		str->c_str[i] = new[i];
	}
	str->c_str[str->len] = '\0';
}

string set_string_from_sv(string_view sv) {
	string tmp = alloc_string(sv.len + 1);
	if(tmp.cap == -1) {
		log_error("Failed to set string content");
		return tmp;
	}
	tmp.len = tmp.cap - 1;
	for(int64_t i = 0; i < tmp.len; i++) {
		tmp.c_str[i] = sv.ptr[i];
	}
	tmp.c_str[tmp.len] = '\0';
	return tmp;
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

string str_append(string str1, string str2) {
	return str_append_c_str(str1, str2.c_str);
}

string str_append_c_str(string str, char* c_str) {
	string_view sv = (string_view) { .ptr = c_str, .len = c_str_len(c_str) };
	return str_append_sv(str, sv);
}

string str_append_sv(string str, string_view sv) {
	if(sv.len <= 0) {
		return str;
	}
	if(str.len + sv.len > str.cap - 1) {
		string tmp = alloc_string(str.cap + sv.len);
		if(tmp.cap < 0) {
			log_error("Failed to append string");
			return (string) { .c_str = NULL, .len = -1, .cap = -1 };
		}
		for(int64_t i = 0; i < str.len; i++) {
			tmp.c_str[i] = str.c_str[i];
		}
		tmp.len = str.len;
		free_string(&str);
		str = tmp;
	}
	for(int64_t i = 0; i < sv.len; i++) {
		str.c_str[str.len + i] = sv.ptr[i];
	}
	str.len = str.len + sv.len;
	str.c_str[str.len] = '\0';
	return str;
}

bool str_cmp(string str_1, string str_2) {
	if(str_1.len != str_2.len) {
		return false;
	}
	bool are_equal = true;
	for(int64_t i = 0; i <= str_1.len; i++) {
		if(str_1.c_str[i] != str_2.c_str[i]) {
			are_equal = false;
			break;
		}
	}
	return are_equal;
}

string str_realloc(string str, int64_t new_cap) {
	// NOTE: the string may shrink due to realloc
	if(new_cap <= 0) {
		return str;
	}
	string tmp = alloc_string(new_cap);
	if(tmp.cap < 0) {
		log_error("Failed to reallocate string");
		return (string) { .c_str = NULL, .len = -1, .cap = -1 };
	}
	int64_t len = 0;
	if(str.len < tmp.cap - 1) {
		len = str.len;
	} else {
		len = tmp.cap - 1;
	}
	for(int64_t i = 0; i < len; i++) {
		tmp.c_str[i] = str.c_str[i];
	}
	free_string(&str);
	tmp.len = len;
	tmp.c_str[tmp.len] = '\0';
	return tmp;
}

bool sv_cmp(string_view sv1, string_view sv2) {
	if(sv1.len != sv2.len) {
		return false;
	}
	bool are_equal = true;
	for(int64_t i = 0; i < sv1.len; i++) {
		if(sv1.ptr[i] != sv2.ptr[i]) {
			are_equal = false;
			break;
		}
	}
	return are_equal;
}

string_view sv_move_forward(string_view *sv, int64_t distance) {
	if(sv == NULL || sv->ptr == NULL) {
		return (string_view) { .ptr = NULL, .len = -1 };
	}
	if(distance >= sv->len) {
		string_view result = (string_view) { .ptr = sv->ptr, .len = sv->len };
		sv->ptr = NULL;
		sv->len = -1;
		return result;
	}
	string_view result = (string_view) { .ptr = sv->ptr, .len = distance };
	sv->ptr = sv->ptr + distance;
	sv->len = sv->len - distance;
	return result;
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

bool sv_trim(string_view* sv) {
	string_view ret_1 = sv_trim_left(sv);
	string_view ret_2 = sv_trim_right(sv);
	if(ret_1.ptr == NULL || ret_2.ptr == NULL) {
		return false;
	}
	return true;
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

string stringify_c_str(char *str) {
	string tmp;
	tmp.c_str = str;
	tmp.len = c_str_len(str);
	tmp.cap = tmp.len + 1;
	return tmp;
}

