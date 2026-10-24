#include <stdio.h>
#include <stdlib.h>

#include "ast.h"
#include "error.h"

static char* builtin_types [] = {
	"char",
	"int",
	"float",
	"double"
};

ast* copy_ast_node(ast* dest, ast* src) {
	if(dest == NULL || src == NULL) {
		return NULL;
	}
	dest->kind = src->kind;
	if(dest->kind == TERMINAL) {
		dest->node.terminal.lexeme = src->node.terminal.lexeme;
		dest->node.terminal.token_kind = src->node.terminal.token_kind;
	} else if(dest->kind == NON_TERMINAL) {
		dest->node.non_terminal = src->node.non_terminal;
	}
	return dest;
}

int64_t ast_child_count(ast* node) {
	if(node == NULL) {
		return -1;
	}
	if(!ast_can_have_children(node)) {
		return 0;
	}
	int64_t num = -1;
	if(node->kind == NON_TERMINAL) {
		num = node->node.non_terminal.children_count;
	}
	return num;
}

int64_t ast_child_cap(ast* node) {
	if(node == NULL) {
		return -1;
	}
	if(!ast_can_have_children(node)) {
		return 0;
	}
	int64_t num = -1;
	if(node->kind == NON_TERMINAL) {
		num = node->node.non_terminal.children_cap;
	}
	return num;
}

ast* ast_nonterminal_insert_child(ast* tree, ast* child) {
	if(tree == NULL) {
		return NULL;
	}
	if(tree->kind != NON_TERMINAL) {
		return NULL;
	}
	if(ast_child_count(tree) >= ast_child_cap(tree)) {
		log_error("Failed to insert child - out of memory");
		return NULL;
	}
	tree->node.non_terminal.children[tree->node.non_terminal.children_count] = child;
	tree->node.non_terminal.children_count++;
	return tree;
}

ast* ast_set_child_array(ast* tree, ast** child_array, int64_t child_count, int64_t child_cap) {
	if(tree == NULL) {
		return NULL;
	}
	if(!ast_can_have_children(tree)) {
		return NULL;
	}
	free(tree->node.non_terminal.children);
	tree->node.non_terminal.children = child_array;
	tree->node.non_terminal.children_count = child_count;
	tree->node.non_terminal.children_cap = child_cap;
	return tree;
}

bool ast_can_have_children(ast* tree) {
	if(tree == NULL) {
		return false;
	}
	if(tree->kind != NON_TERMINAL) {
		return false;
	}
	return true;
}

ast* ast_realloc_children(ast* tree) {
	if(tree == NULL) {
		return NULL;
	}
	if(!ast_can_have_children(tree)) {
		return NULL;
	}

	int64_t new_cap = 5;
	if(ast_child_cap(tree) > new_cap) {
		new_cap = 2 * ast_child_cap(tree);
	}
	ast** tmp = malloc(new_cap * sizeof(*tmp));
	if(tmp == NULL) {
		log_error("Failed to reallocate children of ast");
		return NULL;
	}
	ast_set_child_cap(tree, new_cap);
	for(int64_t i = 0; i < ast_child_count(tree); i++) {
		tmp[i] = get_child(tree, i);
	}
	return ast_set_child_array(tree, tmp, ast_child_count(tree), ast_child_cap(tree));
}

ast* ast_set_child_count(ast* tree, int64_t count) {
	if(!ast_can_have_children(tree)) {
		return NULL;
	}
	tree->node.non_terminal.children_count = count;
	return tree;
}

ast* ast_set_child_cap(ast* tree, int64_t cap) {
	if(!ast_can_have_children(tree)) {
		return NULL;
	}
	tree->node.non_terminal.children_cap = cap;
	return tree;
}

ast* ast_set_last_char_index(ast* tree, int64_t index) {
	if(tree == NULL || index <= 0) {
		return NULL;
	}
	tree->last_char_index = index;
	return tree;
}

int64_t ast_get_last_char_index(ast* tree) {
	if(tree == NULL) {
		return -1;
	}
	return tree->last_char_index;
}

bool ast_add_child(ast* tree, ast* child) {
	if(tree == NULL || child == NULL) {
		return false;
	}
	if(!ast_can_have_children(tree)) {
		return false;
	}
	if(ast_child_count(tree) >= ast_child_cap(tree)) {
		ast* res = ast_realloc_children(tree);
		if(res == NULL) {
			log_error("Failed to add child to ast");
			return false;
		}
	}
	
	ast_set_last_char_index(tree, ast_get_last_char_index(child));
	child->parent = tree;
	
	ast* result = ast_nonterminal_insert_child(tree, child);
	if(result == NULL) {
		log_error("Failed to add child to AST non-terminal node");
		return false;
	}
	return true;
}

int64_t ast_num_tokens_helper(ast* syntax_tree) {
	if(syntax_tree == NULL) {
		return -1;
	}
	int64_t num = 0;
	if(syntax_tree->kind == NON_TERMINAL) {
		for(int64_t i = 0; i < syntax_tree->node.non_terminal.children_count; i++) {
			num += ast_num_tokens_helper(syntax_tree->node.non_terminal.children[i]);
		}
	} else {
		num += 1;
	}
	return num;
}

int64_t ast_num_tokens(ast* syntax_tree) {
	if(syntax_tree == NULL) {
		return -1;
	}
	return ast_num_tokens_helper(syntax_tree);
}

void free_ast_helper(ast* syntax_tree) {
	if(syntax_tree == NULL) {
		return;
	}
	if(syntax_tree->kind == NON_TERMINAL) {
		for(int64_t i = 0; i < syntax_tree->node.non_terminal.children_count; i++) {
			free_ast_helper(syntax_tree->node.non_terminal.children[i]);
		}
		if(is_scope(syntax_tree)) {
			free(syntax_tree->node.non_terminal.extra.type_table.table);
		}
		free(syntax_tree->node.non_terminal.children);
		
	}
	free(syntax_tree);
}

void free_ast(ast* syntax_tree) {
	if(syntax_tree == NULL) {
		return;
	}
	free_ast_helper(syntax_tree);
}

ast* init_ast_syntax_node(syntax_kind kind, int64_t first, int64_t last) {
	ast* node = malloc(sizeof(*node));
	if(node == NULL) {
		log_error("Failed to allocate memory for abstract syntax tree");
		return NULL;
	}
	node->kind = NON_TERMINAL;
	node->parent = NULL;
	node->first_char_index = first;
	node->last_char_index = last;
	node->node.non_terminal = (non_terminal_node) {
		.kind = kind,
		.children = NULL,
		.children_count = -1,
		.children_cap = -1
	};
	return node;
}

ast* init_ast_token_node(token lexical_token) {
	ast* node = malloc(sizeof(*node));
	if(node == NULL) {
		log_error("Failed to allocate memory for abstract syntax tree");
		return NULL;
	}
	node->kind = TERMINAL;
	node->parent = NULL;
	node->node.terminal.lexeme = lexical_token.lexeme;
	node->node.terminal.token_kind = lexical_token.token_kind;
	return node;
}

ast* set_syntax_node(ast** nodes, int64_t num_nodes, syntax_kind kind) {
	ast* syntax_node = init_ast_syntax_node(
		kind,
		nodes[0]->first_char_index,
		nodes[num_nodes - 1]->last_char_index
	);
	if(syntax_node == NULL) {
		for(int64_t i = 0; i < num_nodes; i++) {
			free_ast(nodes[i]);
		}
		return NULL;
	}
	syntax_node->node.non_terminal.children = malloc(num_nodes * sizeof(*syntax_node->node.non_terminal.children));
	if(syntax_node->node.non_terminal.children == NULL) {
		for(int64_t i = 0; i < num_nodes; i++) {
			free_ast(nodes[i]);
		}
		free_ast(syntax_node);
		log_error("Failed to allocate memory for abstract syntax tree");
		return NULL;
	}
	syntax_node->node.non_terminal.children_count = num_nodes;
	for(int64_t i = 0; i < num_nodes; i++) {
		syntax_node->node.non_terminal.children[i] = nodes[i];
	}
	return syntax_node;
	
}

ast* get_child(ast* node, int64_t index) {
	if(node == NULL) {
		return NULL;
	}
	if(node->kind != NON_TERMINAL) {
		return NULL;
	}
	if(index < 0 || index >= ast_child_count(node)) {
		return NULL;
	}
	return node->node.non_terminal.children[index];
}

ast* get_parent(ast* node) {
	if(node == NULL) {
		return NULL;
	}
	return node->parent;
}

bool is_global_scope(ast* node) {
	if(node == NULL) {
		return false;
	}
	if(node->kind != NON_TERMINAL) {
		return false;
	}
	if(node->node.non_terminal.kind != GLOBAL_SCOPE) {
		return false;
	}
	return true;
}

bool is_scope(ast* node) {
	return is_global_scope(node) || is_local_scope(node);
}

bool is_expression(ast* node) {
	if(node == NULL) {
		return false;
	}
	if(node->kind != NON_TERMINAL) {
		return false;
	}
	if(node->node.non_terminal.kind != EXPRESSION) {
		return false;
	}
	return true;
}

bool is_int(ast* node) {
	if(node == NULL) {
		return false;
	}
	if(node->kind != TERMINAL) {
		return false;
	}
	if(node->node.terminal.token_kind != NUMBER) {
		return false;
	}
	if(!sv_is_int_literal(node->node.terminal.lexeme)) {
		return false;
	}
	return true;
}

bool is_function_definition(ast* node) {
	if(node == NULL) {
		return false;
	}
	if(node->kind != NON_TERMINAL) {
		return false;
	}
	if(node->node.non_terminal.kind != FUNCTION_DEFINITION) {
		return false;
	}
	return true;
}

bool is_function_prototype(ast* node) {
	if(node == NULL) {
		return false;
	}
	if(node->kind != NON_TERMINAL) {
		return false;
	}
	if(node->node.non_terminal.kind != FUNCTION_PROTOTYPE) {
		return false;
	}
	return true;
}

bool is_type(ast* node) {
	if(node == NULL) {
		return false;
	}
	if(node->kind != NON_TERMINAL) {
		return false;
	}
	if(node->node.non_terminal.kind != TYPE) {
		return false;
	}
	return true;
}

bool is_local_scope(ast* node) {
	if(node == NULL) {
		return false;
	}
	if(node->kind != NON_TERMINAL) {
		return false;
	}
	if(node->node.non_terminal.kind != LOCAL_SCOPE) {
		return false;
	}
	return true;
}

bool is_return_statement(ast* node) {
	if(node == NULL) {
		return false;
	}
	if(node->kind != NON_TERMINAL) {
		return false;
	}
	if(node->node.non_terminal.kind != RETURN_STATEMENT) {
		return false;
	}
	return true;
}

ast* function_prototype_return_type(ast* fun) {
	if(!is_function_prototype(fun)) {
		return NULL;
	}
	return get_child(fun, 0);
}

ast* function_return_type(ast* fun) {
	if(!is_function_definition(fun)) {
		return NULL;
	}
	return function_prototype_return_type(get_child(fun, 0));
}

ast* function_local_scope(ast* fun) {
	if(!is_function_definition(fun)) {
		return NULL;
	}
	ast* local_scope = NULL;
	for(int64_t i = 0; get_child(fun, i) != NULL; i++) {
		if(is_local_scope(get_child(fun, i))) {
			local_scope = get_child(fun, i);
			break;
		}
	}
	return local_scope;
}

ast* function_return_statement(ast* fun) {
	if(!is_function_definition(fun)) {
		return NULL;
	}
	ast* local_scope = function_local_scope(fun);
	ast* return_statement = NULL;
	for(int64_t i = 0; get_child(local_scope, i) != NULL; i++) {
		if(is_return_statement(get_child(local_scope, i))) {
			// TODO: There can be several returns. Keep a list of them
			return_statement = get_child(local_scope, i);
		}
	}
	return return_statement;
}

ast* function_prototype_name(ast* fun) {
	if(!is_function_prototype(fun)) {
		return NULL;
	}
	return get_child(fun, 1);
}

ast* function_name(ast* fun) {
	if(!is_function_definition(fun)) {
		return NULL;
	}
	return function_prototype_name(get_child(fun, 0));
}

ast* return_statement_expression(ast* statement) {
	if(!is_return_statement(statement)) {
		return NULL;
	}
	return get_child(statement, 1);
}

char* grammar_token_to_c_str(int32_t token) {
	char* str = "AST_ERROR";
	switch(token) {
		case TYPE:
			str = "TYPE";
			break;
		case RETURN_STATEMENT:
			str = "RETURN_STATEMENT";
			break;
		case GLOBAL_SCOPE:
			str = "GLOBAL_SCOPE";
			break;
		case LOCAL_SCOPE:
			str = "LOCAL_SCOPE";
			break;
		case FUNCTION_PROTOTYPE:
			str = "FUNCTION_PROTOTYPE";
			break;
		case FUNCTION_DEFINITION:
			str = "FUNCTION_DEFINITION";
			break;
		case EXPRESSION:
			str = "EXPRESSION";
			break;
		case INVALID_TYPE:
			str = "INVALID_TYPE";
			break;
	}
	return str;
}

void print_ast_helper(ast* syntax_tree, int64_t padding_count) {
	for(int64_t i = 0; i < padding_count; i++) {
		printf("  ");
	}
	if(syntax_tree->kind == NON_TERMINAL) {
		printf("%s\n", grammar_token_to_c_str(syntax_tree->node.non_terminal.kind));
		for(int64_t i = 0; i < syntax_tree->node.non_terminal.children_count; i++) {
			print_ast_helper(syntax_tree->node.non_terminal.children[i], padding_count + 1);
		}
	} else {
		printf("%s - %.*s\n", token_to_c_str(syntax_tree->node.terminal.token_kind), SV_ARG(syntax_tree->node.terminal.lexeme));
	}
}

void print_ast(ast* syntax_tree) {
	if(syntax_tree == NULL) {
		return;
	}
	print_ast_helper(syntax_tree, 0);
}

void print_type_table(type_table tbl) {
	printf("Type table:\nName (id)\n-----------\n");
	for(int64_t i = 0; i < tbl.table_len; i++) {
		printf("%.*s (%ld)\n", SV_ARG(tbl.table[i].name), tbl.table[i].id);
	}
}

bool realloc_type_table(type_table* tbl) {
	int64_t size = 1;
	if(tbl->table_cap > 0) {
		size = 2 * tbl->table_cap;
	}
	type_info* tmp = malloc(size * sizeof(*tmp));
	if(tmp == NULL) {
		log_error("Failed to reallocate memory for type table");
		return false;
	}
	for(int64_t i = 0; i < tbl->table_cap; i++) {
		tmp[i] = tbl->table[i];
	}
	if(tbl->table != NULL) {
		free(tbl->table);
	}
	tbl->table = tmp;
	tbl->table_cap = size;
	return true;
}

type_table create_global_type_table() {
	type_table tbl = (type_table) {
		.table = NULL,
		.table_len = -1,
		.table_cap = -1
	};
	tbl.table = malloc(ARR_LEN(builtin_types) * sizeof(*tbl.table));
	if(tbl.table == NULL) {
		log_error("Failed to allocate memory for type table");
		return (type_table) {
			.table = NULL,
			.table_len = -1,
			.table_cap = -1
		};
	}
	tbl.table_len = 0;
	tbl.table_cap = ARR_LEN(builtin_types);

	for(int64_t i = 0; i < ARR_LEN(builtin_types); i++) {
		if(tbl.table_len >= tbl.table_cap) {
			if(!realloc_type_table(&tbl)) {
				log_error("Failed to create type table");
				return (type_table) {
					.table = NULL,
					.table_len = -1,
					.table_cap = -1
				};
			}
		}
		tbl.table_len++;
		tbl.table[i].id = tbl.table_len;
		tbl.table[i].name = set_sv_ptr(builtin_types[i], c_str_len(builtin_types[i]));
	}
	return tbl;
}
 
bool free_type_table(type_table* tbl) {
	if(tbl->table == NULL) {
		log_error("Attempted double free on type table");
		return false;
	}
	free(tbl->table);
	tbl->table = NULL;
	tbl->table_len = -1;
	tbl->table_len = -1;
	return true;
}

void print_expression_helper(ast* syntax_tree) {
	if(syntax_tree->kind != NON_TERMINAL) {
		return;
	}
	if(is_expression(syntax_tree)) {
		printf("Expr type: %.*s\n", SV_ARG(syntax_tree->node.non_terminal.extra.type.name));
	}
	for(int64_t i = 0; i < syntax_tree->node.non_terminal.children_count; i++) {
		print_expression_helper(syntax_tree->node.non_terminal.children[i]);
	}
}

void print_expression_types(ast* syntax_tree) {
	if(syntax_tree == NULL) {
		return;
	}
	print_expression_helper(syntax_tree);
}

