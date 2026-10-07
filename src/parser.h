#ifndef PARSER_H
#define PARSER_H

#include "pointer.h"

typedef struct abstract_syntax_tree {
	int32_t language_construct;
	struct abstract_syntax_tree** children;
	int64_t children_count;
} ast;

void free_ast(ast* syntax_tree);
char* grammer_token_to_c_str(int32_t token);
ast* parse_function_prototype(int32_ptr tokens, int64_t index);
void print_ast(ast* syntax_tree);

#endif // PARSER_H
