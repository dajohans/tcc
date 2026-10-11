#ifndef POINTER_H
#define POINTER_H

#include <stdint.h>

// NOTE: It may seem kind of stupid to cast to int64_t here, since
// sizeof returns size_t. But not casting leads to lots of warnings
// with -Wextra. One way to get rid of the warnings is to change the
// implementations to use uint64_t but that prevents -1 from being a
// valid error code and fixing that would require changes to the
// architecture. Revisit the idea of switching to uint64_t after
// switching to string_view functions, and in particular after
// replacing next_non_whitespace() by sv_trim_left().
#define ARR_LEN(array) (int64_t)(sizeof(array) / sizeof(array[0]))

typedef struct int32_ptr {
	int32_t* ptr;
	int64_t len;
	int64_t cap;
} int32_ptr;

int32_ptr realloc_int32_ptr(int32_ptr old_ptr);

#endif // POINTER_H
