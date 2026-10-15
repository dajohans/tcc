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

UTEST(string, c_str_len) {
	char str_empty[]  = "";
	char* str_null = NULL;
	char str_nonempty[] = "This string contains 34 characters";
	ASSERT_TRUE(c_str_len(str_empty) == 0);
	ASSERT_TRUE(c_str_len(str_null) == 0);
	ASSERT_TRUE(c_str_len(str_nonempty) == 34);
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
	string_view str_huge = (string_view) { .ptr = NULL, .len = INT64_MAX - 1};
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

/* UTEST(string, append_string) { */
/* 	string str_null = (string) { .c_str = NULL, .len = -1, .cap = -1 }; */
/* 	string result = append_string(str_null, str_null); */
/* 	ASSERT_TRUE(result.c_str == str_null.c_str); */
/* 	ASSERT_TRUE(result.len == str_null.len); */
/* 	ASSERT_TRUE(result.cap == str_null.cap); */

/* 	string str_small = alloc_string(1); */
/* 	string str_huge = (string) { .c_str = NULL, .len = INT64_MAX - 1, .cap = INT64_MAX - 1}; */
/* 	result = append_string(str_small, str_huge); */
/* 	ASSERT_TRUE(result.c_str == str_null.c_str); */
	
/* 	string str_large = alloc_string(5); */
/* 	str_large.len = str_large.cap - 1; */
/* 	result = append_string(str_small, str_large); */
/* 	ASSERT_TRUE(result.cap == str_small.cap + str_large.len); */
/* 	// NOTE: str_small is freed during realloc in append_string */
/* 	free_string(&result); */
/* 	free_string(&str_large); */

	
/* 	str_large = alloc_string(12); */
/* 	str_large.c_str[0] = 'T'; */
/* 	str_large.c_str[1] = 'h'; */
/* 	str_large.c_str[2] = 'i'; */
/* 	str_large.c_str[3] = 's'; */
/* 	str_large.c_str[4] = ' '; */
/* 	str_large.c_str[5] = '\0'; */
/* 	str_large.len = 5; */
/* 	str_small = alloc_string(7); */
/* 	str_small.c_str[0] = 'r'; */
/* 	str_small.c_str[1] = 'o'; */
/* 	str_small.c_str[2] = 'c'; */
/* 	str_small.c_str[3] = 'k'; */
/* 	str_small.c_str[4] = 's'; */
/* 	str_small.c_str[5] = '!'; */
/* 	str_small.c_str[6] = '\0'; */
/* 	str_small.len = 6; */
/* 	str_small.cap = str_small.len + 1; */
/* 	result = append_string(str_large, str_small); */
/* 	// NOTE: append_string() changed the content of the string pointed */
/* 	// to by str_large.c_str, but it doesn't change the len or cap of */
/* 	// str_large. */
/* 	for(int64_t i = 0; i < str_large.len; i++){ */
/* 		ASSERT_TRUE(result.c_str[i] == str_large.c_str[i]); */
/* 	} */
/* 	for(int64_t i = 0; i < str_small.len; i++){ */
/* 		ASSERT_TRUE(result.c_str[str_large.len + i] == str_small.c_str[i]); */
/* 	} */
/* 	free_string(&str_large); */
/* 	free_string(&str_small); */
/* } */

UTEST(string, free_string) {
	string str= alloc_string(10);
	ASSERT_TRUE(str.cap == 10);
	// TODO: Need to adjust string_view.c and string_view.h to use pointer argument for free_string()
	free_string(&str);
	ASSERT_TRUE(str.c_str == NULL);
	ASSERT_TRUE(str.len == -1);
	ASSERT_TRUE(str.cap == -1);
	ASSERT_TRUE(free_string((string*)NULL) == false);
	str.c_str = NULL;
	ASSERT_TRUE(free_string(&str) == false);
}

int main(int argc, const char* const argv[]) {
	return utest_main(argc, argv);
}

