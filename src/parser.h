#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "pointer.h" // Probably remove this later

typedef enum {
	AST_ERROR = -2,
	TYPE = 100,
	RETURN_STATEMENT,
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
	
	struct abstract_syntax_tree** children;
	int64_t children_count;
} syntax_construct;

typedef struct abstract_syntax_tree {
	ast_kind kind;
	union {
		token lex_token;
		syntax_construct syntax;
	} node;
} ast;

ast* parse_rule(token_ptr tokens, int64_t* index, ast* (rule) (token_ptr, int64_t));
ast* parse_top_level(token_ptr tokens, int64_t index);
void free_ast_helper(ast* syntax_tree);
void free_ast(ast* syntax_tree);
char* grammar_token_to_c_str(int32_t token);
ast* init_ast_token_node(token lexical_token);
ast* parse_identifier(token_ptr tokens, int64_t index);
ast* parse_close_curly(token_ptr tokens, int64_t index);
ast* parse_open_curly(token_ptr tokens, int64_t index);
ast* parse_close_paren(token_ptr tokens, int64_t index);
ast* parse_open_paren(token_ptr tokens, int64_t index);
ast* set_syntax_node(ast** nodes, int64_t num_nodes, syntax_kind kind);
ast* parse_keyword_type(token_ptr tokens, int64_t index);
ast* parse_keyword_return(token_ptr tokens, int64_t index);
ast* parse_number(token_ptr tokens, int64_t index);
ast* parse_semicolon(token_ptr tokens, int64_t index);
ast* parse_type(token_ptr tokens, int64_t index);
ast* get_child(ast* node, int64_t index);
bool is_function_definition(ast* node);
bool is_function_prototype(ast* node);
bool is_type(ast* node);
bool is_statement_list(ast* node);
bool is_return_statement(ast* node);
ast* function_return_type(ast* fun);
ast* function_statement_list(ast* fun);
ast* function_return_statement(ast* fun);
ast* function_prototype_name(ast* fun);
ast* function_name(ast* fun);
ast* return_statement_expression(ast* statement);
void print_ast_helper(ast* syntax_tree, int64_t padding_count);
void print_ast(ast* syntax_tree);

#endif // PARSER_H
