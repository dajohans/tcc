#ifndef AST_H
#define AST_H

#include <stdint.h>
#include "string_view.h"
#include "lexer.h"

typedef enum {
	INVALID_TYPE = -2,
	AST_ERROR = -1,
	TYPE = 100,
	RETURN_STATEMENT,
	LOCAL_SCOPE,
	GLOBAL_SCOPE,
	FUNCTION_PROTOTYPE,
	FUNCTION_DEFINITION,
	EXPRESSION
} syntax_kind;

typedef enum {
	TERMINAL, // A lexical token
	NON_TERMINAL // Left-hand side in a grammar rule
} ast_kind;

typedef struct type_info {
	int64_t id;
	string_view name;
} type_info;

typedef struct type_table {
	type_info* table;
	int64_t table_len;
	int64_t table_cap;
	// Possibly add info about where in source it was defined
} type_table;

typedef struct {
	syntax_kind kind;
	union {
		type_table type_table; // For expressions
		type_info type; // For scopes
	} extra;
	
	struct abstract_syntax_tree** children;
	int64_t children_count;
	int64_t children_cap;
} non_terminal_node;

typedef struct terminal_node {
	string_view lexeme;
	int32_t token_kind;
} terminal_node;

typedef struct abstract_syntax_tree {
	ast_kind kind;
	struct abstract_syntax_tree* parent;
	union {
		terminal_node terminal;
		non_terminal_node non_terminal;
	} node;
	int64_t first_char_index;
	int64_t last_char_index;
	// If char* str contains the source code file, then
	// str[first_char_index] is the first character of the source text
	// representation of this AST node and str[last_char_index] is the
	// last character.
} ast;

int64_t ast_child_count(ast* node);
int64_t ast_child_cap(ast* node);
ast* ast_scope_insert_child(ast* tree, ast* child);
ast* ast_nonterminal_insert_child(ast* tree, ast* child);
bool ast_can_have_children(ast* tree);
ast* ast_realloc_children(ast* tree);
ast* ast_set_child_count(ast* tree, int64_t count);
ast* ast_set_child_cap(ast* tree, int64_t cap);
ast* ast_set_last_char_index(ast* tree, int64_t index);
int64_t ast_get_last_char_index(ast* tree);
bool ast_add_child(ast* tree, ast* child);
int64_t ast_num_tokens_helper(ast* syntax_tree);
int64_t ast_num_tokens(ast* syntax_tree);
void free_ast_helper(ast* syntax_tree);
void free_ast(ast* syntax_tree);
ast* init_ast_syntax_node(syntax_kind kind, int64_t first, int64_t last);
ast* init_ast_token_node(token lexical_token);
ast* set_syntax_node(ast** nodes, int64_t num_nodes, syntax_kind kind);
ast* get_child(ast* node, int64_t index);
ast* get_parent(ast* node);
bool is_global_scope(ast* node);
bool is_scope(ast* node);
bool is_expression(ast* node);
bool is_int(ast* node);
bool is_function_definition(ast* node);
bool is_function_prototype(ast* node);
bool is_type(ast* node);
bool is_local_scope(ast* node);
bool is_return_statement(ast* node);
ast* function_return_type(ast* fun);
ast* function_local_scope(ast* fun);
ast* function_return_statement(ast* fun);
ast* function_prototype_name(ast* fun);
ast* function_name(ast* fun);
ast* return_statement_expression(ast* statement);
char* grammar_token_to_c_str(int32_t token);
void print_ast_helper(ast* syntax_tree, int64_t padding_count);
void print_ast(ast* syntax_tree);
void print_type_table(type_table tbl);
bool realloc_type_table(type_table* tbl);
type_table create_global_type_table();
bool free_type_table(type_table* tbl);

#endif // AST_H
