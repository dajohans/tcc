#ifndef ERROR_H
#define ERROR_H

#include <stdio.h>
#include <stdint.h>

extern int64_t debug_trace_depth;

#ifdef DEBUG_VERBOSE
#define DEBUG
#endif // DEBUG VERBOSE

// NOTE C23 is required for __VA_OPT__ 
#define log_error(msg, ...) fprintf(stderr, "[Error] at %s:%d - " msg "\n", __FILE__, __LINE__ __VA_OPT__(,) __VA_ARGS__)

# define log_entry(msg, ...) fprintf(stderr, "[Log] " msg "\n" __VA_OPT__(,) __VA_ARGS__)
// Maybe have log messages of the form
// [type] msg
// for example
// [Log:lexer] found identifier "main" starting at index XXX


// TODO add assertions, perhaps which makes a log entry if the assertion fails

#ifdef DEBUG_VERBOSE
#define log_function_entry(...) do { \
	debug_trace_depth++; \
	fprintf(stderr, "[Debug] "); \
	for(int64_t i = 0; i < debug_trace_depth; i++) { \
		fprintf(stderr, " "); \
	} \
	fprintf(stderr, "%s()\n", __func__); \
} while(0)
#else
#define log_function_entry(...)
#endif

#ifdef DEBUG_VERBOSE
#define log_function_exit(...) do { \
	debug_trace_depth--; \
} while(0)
#else
#define log_function_exit(...)
#endif
// fprintf(stderr, "[Debug] exiting function %s in file %s\n", __func__, __FILE__); \

#endif // ERROR_H
