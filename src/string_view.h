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
bool c_str_cmp(char* str_1, char* str_2);
int64_t c_str_len(char* str);
string concat_string(string str1, string str2);
bool free_string(string* str);
string make_string(char* str);
string set_string(string str, char* new);
string set_string_from_sv(string_view sv);
string_view set_sv(string str, int64_t start_offset, int64_t len);
string_view set_sv_ptr(char* str, int64_t len);
string str_append(string str1, string str2);
string str_append_c_str(string str, char* c_str);
string str_append_sv(string str, string_view sv);
bool str_cmp(string str_1, string str_2);
string str_realloc(string str, int64_t new_cap);
bool sv_cmp(string_view sv1, string_view sv2);
string_view sv_move_forward(string_view *sv, int64_t distance);
string_view sv_split_at_char(string_view *sv, char c);
bool sv_trim(string_view* sv);
string_view sv_trim_left(string_view* sv);
string_view sv_trim_right(string_view* sv);
string stringify_c_str(char *str);

#endif // STRING_H
