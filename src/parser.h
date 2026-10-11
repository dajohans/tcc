#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "pointer.h" // Probably remove this later

typedef enum {
	AST_ERROR = -2,
	TYPE = 100,
	STATEMENT,
	STATEMENT_LIST,
	FUNCTION_PROTOTYPE,
	FUNCTION_DEFINITION
} syntax_kind;

typedef enum {
	TERMINAL, // A lexical token
	NON_TERMINAL // Left-hand side in a grammar rule
} ast_kind;

typedef struct {
	syntax_kind construct;
	int64_t first_char_index;
	int64_t last_char_index;
	
} syntax_construct;

typedef struct abstract_syntax_tree {
	ast_kind kind;
	union {
		token lex_token;
		syntax_construct syntax;
	} node;
	struct abstract_syntax_tree** children;
	int64_t children_count;
} ast;

void free_ast(ast* syntax_tree);
char* grammer_token_to_c_str(int32_t token);
ast* parse_function_definition(token_ptr tokens, int64_t index);
ast* parse_function_prototype(token_ptr tokens, int64_t index);
ast* parse_statement(token_ptr tokens, int64_t index);
ast* parse_type(token_ptr tokens, int64_t index);
void print_ast(ast* syntax_tree);

#endif // PARSER_H
