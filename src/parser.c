#include <stdio.h>
#include <stdlib.h>

#include "error.h"
#include "lexer.h"
#include "parser.h"

/* The goal is to parse the following
 *     int main() {
 *         return 0;
 *     }
 * To to this, I suppose we need the rules:
 *     statement -> keyword_return + number + semicolon
 *     function_prototype -> identifier + identifier + paren_open + paren_close
 *     function_definition -> function_prototype + curly_open + statement + curle_close
 * I think the idea will be to build an abstract syntax tree as
 * follows. Each left-hand side in the grammer rule is a node, and it
 * will have one child for each term in the right-hand side. If the
 * term in the right-hand side is a lexeme such as keyword_return then
 * that child is a leaf node, and if the term is itself a left-hand
 * side of some rule then it will not be a leaf node.
 *
 * All the statements in a given scope will be children of the
 * language construct which owns the scope (if-statement, for-loop,
 * function definition etc). The top level node does not correspond to
 * any language construct in the file and may be thought of as
 * corresponding to the file itself. The children of the top-level
 * node are things like include directives, function declarations and
 * definitions, global variable definitions.
 */

/* TODO: Think about doing tree structures with the following trick
 * from the Linux kernel:
 * #define container_of(ptr, member, type) ((type*)((size_t)ptr-(size_t)&((type*)0)->member))
 * 
 * typedef struct tree {
 *     struct tree* next;
 *     struct tree* prev;
 * } tree;
 * 
 * typedef struct data {
 *     int x;
 *     tree node;
 * } data;
 * 
 * data testdata = (data){ .x = 1, .node = (tree){ .next = NULL, .prev = NULL }};
 * tree* testnode = &testdata.node;
 * printf("test node deref: %d\n", container_of(testnode, node, data)->x);
 */


enum Grammar_Rules {
	AST_ERROR = -2,
	STATEMENT = 100,
	FUNCTION_PROTOTYPE,
	FUNCTION_DEFINITION
};

void free_ast_helper(ast* syntax_tree) {
	if(syntax_tree == NULL) {
		return;
	}
	for(int64_t i = 0; i < syntax_tree->children_count; i++) {
		free_ast_helper(syntax_tree->children[i]);
	}
	free(syntax_tree->children);
	free(syntax_tree);
}

void free_ast(ast* syntax_tree) {
	if(syntax_tree == NULL) {
		return;
	}
	free_ast_helper(syntax_tree);
}

char* grammer_token_to_c_str(int32_t token) {
	char* str = "AST_ERROR";
	switch(token) {
		case STATEMENT:
			str = "STATEMENT";
			break;
		case FUNCTION_PROTOTYPE:
			str = "FUNCTION_PROTOTYPE";
			break;
		case FUNCTION_DEFINITION:
			str = "FUNCTION_DEFINITION";
			break;
	}
	return str;
}

ast* init_ast_node(int32_t language_construct) {
	ast* node = malloc(sizeof(*node));
	if(node == NULL) {
		log_error("Failed to allocate memory for abstract syntax tree");
		return NULL;
	}
	node->language_construct = language_construct;
	node->children = NULL;
	node->children_count = -1;
	return node;
}

ast* parse_function_prototype(int32_ptr tokens, int64_t index) {
	int64_t rule_len = 4;
	if(index + rule_len >= tokens.len){
		return NULL;
	}
	ast* return_type_node = init_ast_node(IDENTIFIER);
	if(return_type_node == NULL) {
		return NULL;
	}
	ast* name_node = init_ast_node(IDENTIFIER);
	if(name_node == NULL) {
		free(return_type_node);
		return NULL;
	}
	ast* paren_open_node = init_ast_node(PAREN_OPEN);
	if(paren_open_node == NULL) {
		free(return_type_node);
		free(name_node);
		return NULL;
	}
	ast* paren_close_node = init_ast_node(PAREN_CLOSE);
	if(paren_close_node == NULL) {
		free(return_type_node);
		free(name_node);
		free(paren_open_node);
		return NULL;
	}
	
	ast* function_prototype_node = init_ast_node(FUNCTION_PROTOTYPE);
	if(function_prototype_node == NULL) {
		free(return_type_node);
		free(name_node);
		free(paren_open_node);
		free(paren_close_node);
		return NULL;
	}
	function_prototype_node->children = malloc(rule_len * sizeof(*function_prototype_node->children));
	if(function_prototype_node->children == NULL) {
		free(return_type_node);
		free(name_node);
		free(paren_open_node);
		free(paren_close_node);
		free(function_prototype_node);
		log_error("Failed to allocate memory for abstract syntax tree");
		return NULL;
	}
	function_prototype_node->children_count = 4;
	function_prototype_node->children[0] = return_type_node;
	function_prototype_node->children[1] = name_node;
	function_prototype_node->children[2] = paren_open_node;
	function_prototype_node->children[3] = paren_close_node;
	return function_prototype_node;
}
 
void print_ast_helper(ast* syntax_tree, int64_t padding_count) {
	for(int64_t i = 0; i < padding_count; i++) {
		printf("  ");
	}
	if(syntax_tree->children_count >= 1) {
		printf("%s\n", grammer_token_to_c_str(syntax_tree->language_construct));
	} else {
		printf("%s\n", token_to_c_str(syntax_tree->language_construct));
	}
	for(int64_t i = 0; i < syntax_tree->children_count; i++) {
		print_ast_helper(syntax_tree->children[i], padding_count + 1);
	}
}

void print_ast(ast* syntax_tree) {
	if(syntax_tree == NULL) {
		return;
	}
	print_ast_helper(syntax_tree, 0);
}

 
 
