#include <stdint.h>

#include "string_view.h"

#include "utest.h"
UTEST_STATE(); // Must be used somewhere
// Source: https://github.com/sheredom/utest.h

UTEST(string, c_str_len) {
	char str_empty[]  = "";
	char* str_null = NULL;
	char str_nonempty[] = "This string is contains 32 characters";
	ASSERT_TRUE(c_str_len(str_empty) == 0);
	ASSERT_TRUE(c_str_len(str_null) == 0);
	ASSERT_TRUE(c_str_len(str_nonempty) == 37);
}

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

UTEST(string, append_string) {
	string str_null = (string) { .c_str = NULL, .len = -1, .cap = -1 };
	string result = append_string(str_null, str_null);
	ASSERT_TRUE(result.c_str == str_null.c_str);
	ASSERT_TRUE(result.len == str_null.len);
	ASSERT_TRUE(result.cap == str_null.cap);

	string str_small = alloc_string(1);
	string str_huge = (string) { .c_str = NULL, .len = INT64_MAX - 1, .cap = INT64_MAX - 1};
	result = append_string(str_small, str_huge);
	ASSERT_TRUE(result.c_str == str_null.c_str);
	
	string str_large = alloc_string(5);
	str_large.len = str_large.cap - 1;
	result = append_string(str_small, str_large);
	ASSERT_TRUE(result.cap == str_small.cap + str_large.len);
	// NOTE: str_small is freed during realloc in append_string
	free_string(&result);
	free_string(&str_large);

	
	str_large = alloc_string(12);
	str_large.c_str[0] = 'T';
	str_large.c_str[1] = 'h';
	str_large.c_str[2] = 'i';
	str_large.c_str[3] = 's';
	str_large.c_str[4] = ' ';
	str_large.c_str[5] = '\0';
	str_large.len = 5;
	str_small = alloc_string(7);
	str_small.c_str[0] = 'r';
	str_small.c_str[1] = 'o';
	str_small.c_str[2] = 'c';
	str_small.c_str[3] = 'k';
	str_small.c_str[4] = 's';
	str_small.c_str[5] = '!';
	str_small.c_str[6] = '\0';
	str_small.len = 6;
	str_small.cap = str_small.len + 1;
	result = append_string(str_large, str_small);
	// NOTE: append_string() changed the content of the string pointed
	// to by str_large.c_str, but it doesn't change the len or cap of
	// str_large.
	for(int64_t i = 0; i < str_large.len; i++){
		ASSERT_TRUE(result.c_str[i] == str_large.c_str[i]);
	}
	for(int64_t i = 0; i < str_small.len; i++){
		ASSERT_TRUE(result.c_str[str_large.len + i] == str_small.c_str[i]);
	}
	free_string(&str_large);
	free_string(&str_small);
}

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

