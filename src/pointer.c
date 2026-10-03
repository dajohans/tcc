#include <stdlib.h>

#include "error.h"
#include "pointer.h"

int32_ptr realloc_int32_ptr(int32_ptr old_ptr) {
	int32_ptr new_ptr;
	new_ptr.ptr = malloc(2 * old_ptr.cap * sizeof(*new_ptr.ptr));
	if(new_ptr.ptr == NULL) {
		log_error("Failed to allocate memory for int32_ptr");
		free(old_ptr.ptr);
		return (int32_ptr){ .ptr = NULL, .len = -1, .cap = -1 };
	}
	new_ptr.cap = 2 * old_ptr.cap;
	new_ptr.len = old_ptr.len;
	for(int64_t j = 0; j < old_ptr.len; j++) {
		new_ptr.ptr[j] = old_ptr.ptr[j];
	}
	free(old_ptr.ptr);
	return new_ptr;
}
