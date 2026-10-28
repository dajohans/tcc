#include <stdint.h>

#include "string_view.h"

#include "../lib/utest/utest.h"
UTEST_STATE(); // Must be used somewhere
// Source: https://github.com/sheredom/utest.h

UTEST(string, alloc_string) {
	string str_null = (string) { .c_str = NULL, .len = -1, .cap = -1 };
	string str_bad_alloc_1 = alloc_string(-1);
	string str_bad_alloc_2 = alloc_string(0);
	string str_bad_alloc_3 = alloc_string(INT64_MAX);
	string str_good = alloc_string(10);
	
	ASSERT_TRUE(str_bad_alloc_1.len == str_null.len);
	ASSERT_TRUE(str_bad_alloc_1.cap == str_null.cap);
	
	ASSERT_TRUE(str_bad_alloc_2.len == str_null.len);
	ASSERT_TRUE(str_bad_alloc_2.cap == str_null.cap);
	
	ASSERT_TRUE(str_bad_alloc_3.len == str_null.len);
	ASSERT_TRUE(str_bad_alloc_3.cap == str_null.cap);
	
	ASSERT_TRUE(str_good.len == 0);
	ASSERT_TRUE(str_good.cap == 10);
	
	free_string(&str_bad_alloc_1);
	free_string(&str_bad_alloc_2);
	free_string(&str_good);
}

UTEST(string, c_str_cmp) {
	// Test early return
	ASSERT_TRUE(c_str_cmp(NULL, "test") == false);
	// Test non-equal strings
	ASSERT_TRUE(c_str_cmp("failure", "test") == false);
	// Test equal strings
	ASSERT_TRUE(c_str_cmp(NULL, NULL) == true);
	ASSERT_TRUE(c_str_cmp("test", "test") == true);
}

UTEST(string, c_str_len) {
	char str_empty[]  = "";
	char* str_null = NULL;
	char str_nonempty[] = "This string contains 34 characters";
	ASSERT_TRUE(c_str_len(str_empty) == 0);
	ASSERT_TRUE(c_str_len(str_null) == 0);
	ASSERT_TRUE(c_str_len(str_nonempty) == 34);
}

UTEST(string, concat_string) {
	string str_null = (string) { .c_str = NULL, .len = -1, .cap = -1 };
	string str_short = make_string("This");
	string result;
	// Test first early return
	result = concat_string(str_null, str_null);
	ASSERT_TRUE(result.cap == -1);
	// Test second early return
	result = concat_string(str_null, str_short);
	ASSERT_TRUE(str_cmp(result, str_short) == true);
	ASSERT_TRUE(result.len == str_short.len);
	ASSERT_TRUE(result.cap == str_short.cap);
	// Test second early return
	result = concat_string(str_short, str_null);
	ASSERT_TRUE(str_cmp(result, str_short) == true);
	ASSERT_TRUE(result.len == str_short.len);
	ASSERT_TRUE(result.cap == str_short.cap);
	// Test failed allocation
	string str_less_huge = (string) { .c_str = NULL, .len = -1, .cap = 1 };
	string str_huge = (string) { .c_str = NULL, .len = INT64_MAX, .cap = 1 };
	result = concat_string(str_less_huge, str_huge);
	ASSERT_TRUE(result.cap == -1);
	// Test successful concatenation
	string ending = make_string(" is a test");
	string test = make_string("This is a test");
	result = concat_string(str_short, ending);
	ASSERT_TRUE(str_cmp(result, test) == true);
	ASSERT_TRUE(result.len == test.len);
	ASSERT_TRUE(result.cap == test.cap);
	ASSERT_TRUE(result.c_str[result.len] == '\0');
	
	free_string(&result);
	free_string(&test);
	free_string(&ending);
	free_string(&str_short);
}

UTEST(string, free_string) {
	string* null_ptr = NULL;
	string str_null = (string) { .c_str = NULL, .len = -1, .cap = -1 };
	// Test early returns
	ASSERT_TRUE(free_string(null_ptr) == false);
	ASSERT_TRUE(free_string(&str_null) == false);

	string str= alloc_string(10);
	ASSERT_TRUE(str.cap == 10);
	// Test successful free
	ASSERT_TRUE(free_string(&str) == true);
	ASSERT_TRUE(str.c_str == NULL);
	ASSERT_TRUE(str.len == -1);
	ASSERT_TRUE(str.cap == -1);
}

UTEST(string, make_string) {
	string result;
	// Test first early return
	result = make_string(NULL);
	ASSERT_TRUE(result.cap == -1);
	char* c_str = "test";
	string str = make_string(c_str);
	ASSERT_TRUE(str.len == c_str_len(c_str));
	ASSERT_TRUE(str.cap == c_str_len(c_str) + 1);
	ASSERT_TRUE(c_str_cmp(str.c_str, c_str));
	free_string(&str);
}

UTEST(string, set_string) {
	string str_null = (string) { .c_str = NULL, .len = -1, .cap = -1 };
	string result = set_string(str_null, NULL);
	// Test early return
	ASSERT_TRUE(str_cmp(result, str_null));
	free_string(&result);
	// Test assignment without reallocation
	string str_long = make_string("This is a long sentence.");
	result = set_string(str_long, "Hello there");
	ASSERT_TRUE(str_cmp(str_long, result) != true);
	ASSERT_TRUE(c_str_cmp(str_long.c_str, result.c_str) == true);
	ASSERT_TRUE(str_long.len != result.len);
	ASSERT_TRUE(str_long.cap == result.cap);
	// Note: str_cmp fails on early return, since str_long did not get
	// its .len updated
	
	free_string(&str_long);
}

UTEST(string, set_string_from_sv) {
	string_view sv_null = (string_view) { .ptr = NULL, .len = -1 };
	// Test early return
	string result = set_string_from_sv(sv_null);
	ASSERT_TRUE(result.c_str == NULL);
	ASSERT_TRUE(result.len == -1);
	ASSERT_TRUE(result.cap == -1);
	// Test empty string_view
	string_view sv_empty = (string_view) { .ptr = NULL, .len = 0 };
	result = set_string_from_sv(sv_empty);
	ASSERT_TRUE(result.c_str[0] == '\0');
	ASSERT_TRUE(result.len == 0);
	ASSERT_TRUE(result.cap == 1);
	free_string(&result);
	// Test non-empty string_view
	char* str = "test string";
	string_view sv_short = (string_view) { .ptr = str, .len = c_str_len(str)};
	result = set_string_from_sv(sv_short);
	ASSERT_TRUE(c_str_cmp(result.c_str, str));
	ASSERT_TRUE(result.len == sv_short.len);
	ASSERT_TRUE(result.cap == sv_short.len + 1);
	free_string(&result);
}

UTEST(string, set_sv) {
	string str_null = (string) { .c_str = NULL, .len = -1, .cap = -1 };
	string_view result;
	// Test first early return
	result = set_sv(str_null, 1, 1);
	ASSERT_TRUE(result.ptr == NULL);
	ASSERT_TRUE(result.len == -1);
	// Test second early return
	string str_short = make_string("0123");
	result = set_sv(str_short, 1, str_short.cap);
	ASSERT_TRUE(result.len == -1);
	free_string(&str_short);
	// Test successful assignment in middle of string
	string str = make_string("1234567890");
	string_view sv = (string_view) { .ptr = "67", .len = 2};
	result = set_sv(str, 5, 2);
	ASSERT_TRUE(sv_cmp(result, sv));
	// Test successful assignment up to and including terminating null
	sv = (string_view) { .ptr = "0", .len = 2};
	result = set_sv(str, 9, 2);
	ASSERT_TRUE(sv_cmp(result, sv) == true);
	
	free_string(&str);
}

UTEST(string, set_sv_ptr) {
	// Test first early return
	string_view result = set_sv_ptr(NULL, 0);
	ASSERT_TRUE(result.ptr == NULL);
	ASSERT_TRUE(result.len == -1);
	// Test trying to set 1 index past terminating null
	result = set_sv_ptr("123", 5);
	ASSERT_TRUE(result.ptr == NULL);
	ASSERT_TRUE(result.len == -1);
	// Test successful assignment in the middle of string
	string_view sv = (string_view) { .ptr = "12", .len = 2 };
	char* str = "12345";
	result = set_sv_ptr(str, 2);
	ASSERT_TRUE(sv_cmp(result, sv) == true);
	// Test successful assignment up to and including terminating null
	sv = (string_view) { .ptr = str, .len = c_str_len(str) + 1 };
	result = set_sv_ptr(str, c_str_len(str) + 1);
	ASSERT_TRUE(sv_cmp(result, sv) == true);
}

UTEST(string, str_append_c_str) {
	// Test append null string
	char* cstr = NULL;
	string str = make_string("Test string");
	string result = str_append_c_str(str, cstr);
	ASSERT_TRUE(str_cmp(str, result) == true);
	ASSERT_TRUE(result.cap == str.cap);
	free_string(&str);
	// Test append empty string
	cstr = "";
	str = make_string("Test string");
	result = str_append_c_str(str, cstr);
	ASSERT_TRUE(str_cmp(str, result) == true);
	ASSERT_TRUE(result.cap == str.cap);
	free_string(&str);
}

UTEST(string, str_append_sv) {
	// Test first early return
	string str_null = (string) { .c_str = NULL, .len = -1, .cap = -1 };
	string_view sv_null = (string_view) { .ptr = NULL, .len = -1 };
	string result = str_append_sv(str_null, sv_null);
	ASSERT_TRUE(result.c_str == str_null.c_str);
	ASSERT_TRUE(result.len == str_null.len);
	ASSERT_TRUE(result.cap == str_null.cap);

	// Test early return in reallocation
	string str_small = alloc_string(1);
	string_view str_huge = (string_view) { .ptr = "test", .len = INT64_MAX - 1};
	result = str_append_sv(str_small, str_huge);
	ASSERT_TRUE(result.c_str == NULL);
	
	// Test successful reallocation
	string_view sv_large = (string_view) { .ptr = "test", .len = 4 };
	result = str_append_sv(str_small, sv_large);
	ASSERT_TRUE(result.cap == str_small.cap + sv_large.len);
	ASSERT_TRUE(result.c_str[0] == 't');
	ASSERT_TRUE(result.c_str[1] == 'e');
	ASSERT_TRUE(result.c_str[2] == 's');
	ASSERT_TRUE(result.c_str[3] == 't');
	ASSERT_TRUE(result.c_str[4] == '\0');
	// NOTE: str_small is freed during realloc in str_append
	free_string(&result);

	// Test successful appending without reallocation
	str_small = alloc_string(12);
	str_small.c_str[0] = 'T';
	str_small.c_str[1] = 'h';
	str_small.c_str[2] = 'i';
	str_small.c_str[3] = 's';
	str_small.c_str[4] = ' ';
	str_small.c_str[5] = '\0';
	str_small.len = 5;
	sv_large.ptr = "rocks!";
	sv_large.len = 6;
	result = str_append_sv(str_small, sv_large);
	// NOTE: str_append() changed the content of the string pointed to
	// by str_small.c_str, but it doesn't change the len or cap of
	// str_small.
	for(int64_t i = 0; i < str_small.len; i++){
		ASSERT_TRUE(result.c_str[i] == str_small.c_str[i]);
	}
	for(int64_t i = 0; i < sv_large.len; i++){
		ASSERT_TRUE(result.c_str[str_small.len + i] == sv_large.ptr[i]);
	}
	free_string(&str_small);
}

UTEST(string, str_cmp) {
	string str1 = make_string("test");
	string str2 = (string) { .c_str = NULL, .len = -1, .cap = -1 };
	ASSERT_TRUE(str_cmp(str1, str2) == false);
	ASSERT_TRUE(str_cmp(str2, str1) == false);
	
	str2 = make_string("nope");
	ASSERT_TRUE(str_cmp(str1, str2) == false);

	// Test success with same capacity
	string str3 = make_string("test");
	ASSERT_TRUE(str_cmp(str1, str3) == true);
	
	// Test success with same content but different capacity
	string str4 = alloc_string(10);
	str4.c_str[0] = 't';
	str4.c_str[1] = 'e';
	str4.c_str[2] = 's';
	str4.c_str[3] = 't';
	str4.c_str[4] = '\0';
	str4.len = c_str_len(str4.c_str);
	ASSERT_TRUE(str_cmp(str1, str4) == true);
	
	// Test failure when only terminating null is different
	string str5 = make_string("test");
	string str6 = make_string("test");
	str6.c_str[str6.len] = 'f';
	ASSERT_TRUE(str_cmp(str5, str6) == false);

	free_string(&str1);
	free_string(&str2);
	free_string(&str3);
	free_string(&str4);
	free_string(&str5);
	free_string(&str6);
}

UTEST(string, str_realloc) {
	string str_null = (string) { .c_str = NULL, .len = -1, .cap = -1 };
	string result;
	// Test no reallocation if invalid new capacity
	result = str_realloc(str_null, -1);
	ASSERT_TRUE(str_null.c_str == result.c_str);
	ASSERT_TRUE(str_null.len == result.len);
	ASSERT_TRUE(str_null.cap == result.cap);
	
	// Test early return due to allocation failure
	result = str_realloc(str_null, INT64_MAX);
	ASSERT_TRUE(str_null.c_str == result.c_str);
	ASSERT_TRUE(str_null.len == result.len);
	ASSERT_TRUE(str_null.cap == result.cap);
	
	// Test that failed allocation did not mess up original string
	string str = make_string("test string");
	string str_old = alloc_string(str.cap);
	for(int64_t i = 0; i < str.cap; i++) {
		str_old.c_str[i] = str.c_str[i];
	}
	str_old.len = str.len;
	ASSERT_TRUE(str_cmp(str, str_old) == true);
	free_string(&str);
	free_string(&str_old);

	// Test successfull reallocation that shrinks the string
	string str_long = make_string("test string");
	str_old = str_long;
	string str_short = make_string("test");
	str_long = str_realloc(str_long, str_short.cap);
	ASSERT_TRUE(str_long.len == str_short.len);
	ASSERT_TRUE(str_long.cap == str_short.cap);
	ASSERT_TRUE(str_cmp(str_long, str_short) == true);
	ASSERT_TRUE(str_long.c_str != str_old.c_str);
	free_string(&str_long);
	free_string(&str_short);
	
	// Test successfull reallocation that grows the string
	str_long = make_string("test string");
	str_old = str_long;
	string str_same = make_string(str_long.c_str);
	str_long = str_realloc(str_long, str_long.cap + 1);
	ASSERT_TRUE(str_long.len == str_same.len);
	ASSERT_TRUE(str_long.cap == str_same.cap + 1);
	ASSERT_TRUE(str_cmp(str_long, str_same) == true);
	ASSERT_TRUE(str_long.c_str != str_old.c_str);
	free_string(&str_long);
	free_string(&str_same);
}

UTEST(string, sv_cmp) {
	string_view sv_null = (string_view) { .ptr = NULL, .len = -1 };
	string_view sv_short = (string_view) { .ptr = "test", .len = c_str_len("test") };
	string_view sv_long = (string_view) { .ptr = "failure", .len = c_str_len("failure") };
	// Test early return
	ASSERT_TRUE(sv_cmp(sv_null, sv_short) == false);
	// Test non-equal strings
	ASSERT_TRUE(sv_cmp(sv_long, sv_short) == false);
	// Test equal strings
	ASSERT_TRUE(sv_cmp(sv_null, sv_null) == true);
	ASSERT_TRUE(sv_cmp(sv_short, sv_short) == true);
}

UTEST(string, sv_move_forward) {
	string_view sv_null = (string_view) { .ptr = NULL, .len = -1 };
	string_view result = sv_move_forward((string_view*)NULL, 0);
	// Test first early return
	ASSERT_TRUE(result.len == -1);
	result = sv_move_forward(&sv_null, 0);
	ASSERT_TRUE(result.len == -1);
	// Attempt to move past the end, resulting in moving precisely to the end
	string_view sv = (string_view) { .ptr = "test", .len = c_str_len("test") };
	string_view sv_orig = (string_view) { .ptr = sv.ptr, .len = sv.len };
	result = sv_move_forward(&sv, sv.len + 10);
	ASSERT_TRUE(sv.ptr == NULL);
	ASSERT_TRUE(sv.len == -1);
	ASSERT_TRUE(result.ptr == sv_orig.ptr);
	ASSERT_TRUE(result.len == sv_orig.len);
	// Test moving halfway through the string_view
	sv = (string_view) { .ptr = "test", .len = c_str_len("test") };
	string_view expected_modification = (string_view) { .ptr = sv.ptr + 2, .len = sv.len - 2 };
	string_view expected_return = (string_view) { .ptr = sv.ptr,.len = 2 };
	result = sv_move_forward(&sv, 2);
	ASSERT_TRUE(sv.ptr == expected_modification.ptr);
	ASSERT_TRUE(sv.len == expected_modification.len);
	ASSERT_TRUE(result.ptr == expected_return.ptr);
	ASSERT_TRUE(result.len == expected_return.len);
}

UTEST(string, sv_split_at_char) {
	string_view sv_null = (string_view) { .ptr = NULL, .len = -1 };
	string_view result = sv_split_at_char((string_view*)NULL, 'a');
	// Test first early return
	ASSERT_TRUE(result.len == -1);
	result = sv_split_at_char(&sv_null, 'a');
	ASSERT_TRUE(result.len == -1);
	// Test split in the middle
	string_view sv = (string_view) { .ptr = "test", .len = c_str_len("test") };
	string_view sv_orig = (string_view) { .ptr = sv.ptr, .len = sv.len };
	string_view expected_modification = (string_view) { .ptr = "st", .len = c_str_len("st") };
	string_view expected_return = (string_view) { .ptr = "te", .len = c_str_len("te") };
	result = sv_split_at_char(&sv, 's');
	ASSERT_TRUE(result.ptr == sv_orig.ptr);
	ASSERT_TRUE(sv_cmp(result, expected_return));
	ASSERT_TRUE(sv.ptr == sv_orig.ptr + 2);
	ASSERT_TRUE(sv_cmp(sv, expected_modification));
	// Test split at nonexistent character
	sv = (string_view) { .ptr = "test", .len = c_str_len("test") };
	sv_orig = (string_view) { .ptr = sv.ptr, .len = sv.len };
	result = sv_split_at_char(&sv, 'a');
	ASSERT_TRUE(result.ptr == sv_orig.ptr);
	ASSERT_TRUE(sv_cmp(result, sv_orig) == true);
	ASSERT_TRUE(sv.ptr == NULL);
	ASSERT_TRUE(sv.len == -1);
}

UTEST(string, sv_trim_left) {
	string_view sv_null = (string_view) { .ptr = NULL, .len = -1 };
	string_view result = sv_trim_left((string_view*)NULL);
	// Test first early return
	ASSERT_TRUE(result.len == -1);
	result = sv_trim_left(&sv_null);
	ASSERT_TRUE(result.len == -1);
	// Test split string of only whitespace
	char* whitespace = "      ";
	string_view sv = (string_view) { .ptr = whitespace, .len = c_str_len(whitespace) };
	string_view sv_orig = (string_view) { .ptr = sv.ptr, .len = sv.len };
	result = sv_trim_left(&sv);
	ASSERT_TRUE(sv.ptr[0] == '\0');
	ASSERT_TRUE(sv.len == 0);
	ASSERT_TRUE(result.ptr == sv_orig.ptr);
	ASSERT_TRUE(result.len == sv_orig.len);
	// Test without whitespace to trim
	char* no_whitespace = "There is no whitespace to trim";
	sv = (string_view) { .ptr = no_whitespace, .len = c_str_len(no_whitespace) };
	sv_orig = (string_view) { .ptr = sv.ptr, .len = sv.len };
	result = sv_trim_left(&sv);
	ASSERT_TRUE(result.ptr == sv.ptr);
	ASSERT_TRUE(result.len == 0);
	ASSERT_TRUE(sv.ptr == sv_orig.ptr);
	ASSERT_TRUE(sv.len == sv_orig.len);
	// Test with whitespace to trim
	whitespace = " There is whitespace to trim";
	sv = (string_view) { .ptr = whitespace, .len = c_str_len(whitespace) };
	sv_orig = (string_view) { .ptr = sv.ptr, .len = sv.len };
	result = sv_trim_left(&sv);
	ASSERT_TRUE(result.ptr == sv_orig.ptr);
	ASSERT_TRUE(result.len == 1);
	ASSERT_TRUE(sv.ptr == sv_orig.ptr + 1);
	ASSERT_TRUE(sv.len == sv_orig.len - 1);
}

UTEST(string, sv_trim_right) {
	string_view sv_null = (string_view) { .ptr = NULL, .len = -1 };
	string_view result = sv_trim_right((string_view*)NULL);
	// Test first early return
	ASSERT_TRUE(result.len == -1);
	result = sv_trim_right(&sv_null);
	ASSERT_TRUE(result.len == -1);
	// Test split string of only whitespace
	char* whitespace = "      ";
	string_view sv = (string_view) { .ptr = whitespace, .len = c_str_len(whitespace) };
	string_view sv_orig = (string_view) { .ptr = sv.ptr, .len = sv.len };
	result = sv_trim_right(&sv);
	ASSERT_TRUE(sv.ptr == sv_orig.ptr);
	ASSERT_TRUE(sv.len == 0);
	ASSERT_TRUE(result.ptr == sv_orig.ptr);
	ASSERT_TRUE(result.len == sv_orig.len);
	// Test without whitespace to trim
	char* no_whitespace = "There is no whitespace to trim";
	sv = (string_view) { .ptr = no_whitespace, .len = c_str_len(no_whitespace) };
	sv_orig = (string_view) { .ptr = sv.ptr, .len = sv.len };
	result = sv_trim_right(&sv);
	ASSERT_TRUE(result.ptr[0] == '\0');
	ASSERT_TRUE(result.len == 0);
	ASSERT_TRUE(sv.ptr == sv_orig.ptr);
	ASSERT_TRUE(sv.len == sv_orig.len);
	// Test with whitespace to trim
	whitespace = "There is whitespace to trim ";
	sv = (string_view) { .ptr = whitespace, .len = c_str_len(whitespace) };
	sv_orig = (string_view) { .ptr = sv.ptr, .len = sv.len };
	result = sv_trim_right(&sv);
	ASSERT_TRUE(result.ptr[1] == '\0');
	ASSERT_TRUE(result.len == 1);
	ASSERT_TRUE(sv.ptr == sv_orig.ptr);
	ASSERT_TRUE(sv.len == sv_orig.len - 1);
}

UTEST(string, stringify_c_str) {
	// Test early return
	char* cstr = NULL;
	string result = stringify_c_str(cstr);
	ASSERT_TRUE(result.c_str == NULL);
	ASSERT_TRUE(result.len == -1);
	ASSERT_TRUE(result.cap == -1);
	// Test valid string of length zero
	cstr = "";
	result = stringify_c_str(cstr);
	ASSERT_TRUE(result.c_str == cstr);
	ASSERT_TRUE(result.len == 0);
	ASSERT_TRUE(result.cap == 1);
	// Test valid string of positive length
	cstr = "This is a string";
	result = stringify_c_str(cstr);
	ASSERT_TRUE(result.c_str == cstr);
	ASSERT_TRUE(result.len == c_str_len(cstr));
	ASSERT_TRUE(result.cap == c_str_len(cstr) + 1);
}

int main(int argc, const char* const argv[]) {
	return utest_main(argc, argv);
}

