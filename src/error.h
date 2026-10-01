#ifndef ERROR_H
#define ERROR_H

#include <stdio.h>

// NOTE C23 is required for __VA_OPT__ 
#define log_error(msg, ...) fprintf(stderr, "[Error] at %s:%d - " msg "\n", __FILE__, __LINE__ __VA_OPT__(,) __VA_ARGS__)

#endif // ERROR_H
