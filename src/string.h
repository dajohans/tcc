#ifndef STRING_H
#define STRING_H

#include <stdint.h>

#define C_STR_LEN(str) ((sizeof(str) / sizeof(str[0])) - 1)

typedef struct string {
	char* c_str;
	int64_t len;
	int64_t cap;
} string;

// TODO: Since the string_view functions modify the pointer parameter,
// they cannot be applied to a static string. So we cannot call
// sv_trim("  hej  "). Think about if this is desirable or not...
// Maybe implement another function without pointers and do function
// overloading with _Generic.
typedef struct string_view {
	char* ptr;
	int64_t len;
	// TODO: Maybe add info about the surrounding string, so we can
	// make sure that the string_view does not try to peek outside of
	// the string.
} string_view;

#define SV_ARG(sv) (int)sv.len, sv.ptr

string alloc_string(int64_t cap);
string append_string(string str, string end);
int64_t c_str_len(char* str);
string concat_string(string str1, string str2);
void free_string(string str);
string set_string(char* str);
string_view set_sv(string str, int64_t start_offset, int64_t len);
string_view set_sv_ptr(char* str, int64_t len);
bool str_cmp(string str_1, string str_2);
string_view sv_split_at_char(string_view *sv, char c);
void sv_trim(string_view* sv);
string_view sv_trim_left(string_view* sv);
string_view sv_trim_right(string_view* sv);
string stringify_c_str(char *str);

#endif // STRING_H
