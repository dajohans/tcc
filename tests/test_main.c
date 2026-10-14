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
	string str_bad_alloc_3 = alloc_string(SIZE_MAX);
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

