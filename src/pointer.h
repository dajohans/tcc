#ifndef POINTER_H
#define POINTER_H

#include <stdint.h>

#define ARR_LEN(array) (sizeof(array) / sizeof(array[0]))

typedef struct int32_ptr {
	int32_t* ptr;
	int64_t len;
	int64_t cap;
} int32_ptr;

int32_ptr realloc_int32_ptr(int32_ptr old_ptr);

#endif // POINTER_H
