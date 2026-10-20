#include <stdio.h>
#include <stdlib.h>

#include "error.h"
#include "lexer.h"
#include "parser.h"

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

ast* grammar_end(token_ptr, int64_t) { return NULL; }
ast* parse_return_statement(token_ptr, int64_t) { return NULL; }
ast* parse_function_prototype(token_ptr, int64_t) { return NULL; }
ast* parse_function_definition(token_ptr, int64_t) { return NULL; }
ast* parse_statement_list(token_ptr, int64_t) { return NULL; }


/* a grammar rule with several alternatives such as
 * 'type -> keyword_int OR keyword_float'
 * is represented by two arrays; one array for each alternative. So we
 * would have one array for 'type -> keyword_int' and one for
 * 'type -> keyword_float'.
 *
 * A grammar rule with arbitrary amount of repetition such as 
 * 'statement_list -> statement*'
 * is represented by a grammar rule which ends with the function
 * grammar_repeat.
 */

#define RULE_LEN 10
ast* (*grammar[][RULE_LEN]) (token_ptr, int64_t) = {
	{ parse_type,                parse_keyword_type,                                                                  grammar_end, grammar_end, grammar_end, grammar_end, grammar_end, grammar_end, grammar_end, grammar_end },
	{ parse_return_statement,    parse_keyword_return, parse_number, parse_semicolon,                                 grammar_end, grammar_end, grammar_end, grammar_end, grammar_end, grammar_end },
	{ parse_function_prototype,  parse_type, parse_identifier, parse_open_paren, parse_close_paren,                   grammar_end, grammar_end, grammar_end, grammar_end, grammar_end  },
	{ parse_function_definition, parse_function_prototype, parse_open_curly, parse_statement_list, parse_close_curly, grammar_end, grammar_end, grammar_end, grammar_end, grammar_end  },
	{ parse_statement_list,      parse_return_statement,                                                              grammar_end, grammar_end, grammar_end, grammar_end, grammar_end, grammar_end, grammar_end, grammar_end  }
};

syntax_kind rule_kind[] = {
	TYPE,
	RETURN_STATEMENT,
	FUNCTION_PROTOTYPE,
	FUNCTION_DEFINITION,
	STATEMENT_LIST
};

bool is_grammar_rule(ast* (rule) (token_ptr, int64_t)) {
	bool found = false;
	for(int64_t l = 0; l < ARR_LEN(grammar); l++) {
		if(rule == grammar[l][0]) {
			found = true;
			break;
		}
	}
	return found;
}

bool ast_add_child(ast* tree, ast* child) {
	if(tree->kind == TERMINAL) {
		return false;
	}
	if(tree->node.syntax.children_count == -1) {
		tree->node.syntax.children = malloc(RULE_LEN * sizeof(*tree->node.syntax.children));
		if(tree->node.syntax.children == NULL) {
			log_error("Failed memory allocation while adding AST child node");
			return false;
		}
		tree->node.syntax.children_count = 0;
		if(child->kind == TERMINAL) {
			tree->node.syntax.first_char_index = child->node.lex_token.first_char_index;
		} else {
			tree->node.syntax.first_char_index = child->node.syntax.first_char_index;
		}
	}
	if(child->kind == TERMINAL) {
		tree->node.syntax.last_char_index = child->node.lex_token.last_char_index;
	} else {
		tree->node.syntax.last_char_index = child->node.syntax.last_char_index;
	}
	tree->node.syntax.children[tree->node.syntax.children_count] = child;
	tree->node.syntax.children_count++;
	return true;
}

int64_t ast_num_tokens_helper(ast* syntax_tree) {
	if(syntax_tree == NULL) {
		return -1;
	}
	int64_t num = 0;
	if(syntax_tree->kind == NON_TERMINAL) {
		for(int64_t i = 0; i < syntax_tree->node.syntax.children_count; i++) {
			num += ast_num_tokens_helper(syntax_tree->node.syntax.children[i]);
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

ast* parse_rule(token_ptr tokens, int64_t* index, ast* (rule) (token_ptr, int64_t)) {
	if(*index >= tokens.len) {
		return NULL;
	}
	syntax_kind current_rule_kind = AST_ERROR;
	for(int64_t i = 0; i < ARR_LEN(grammar); i++) {
		if(rule == grammar[i][0]) {
			current_rule_kind = rule_kind[i];
			break;
		}
	}
	if(current_rule_kind == AST_ERROR) {
		return NULL;
	}
	
	int64_t num_nodes = 0;
	ast *nodes[RULE_LEN - 1]; // -1 since the first element in each grammar rule is a placeholder
	ast* top_node = malloc(sizeof(*top_node));
	if(top_node == NULL) {
		return NULL;
	}
	top_node->kind = NON_TERMINAL;
	top_node->node.syntax = (syntax_construct) {
		.construct = current_rule_kind,
		.first_char_index = -1,
		.last_char_index = -1,
		.children = NULL,
		.children_count = -1
	};
	bool successful_rule_parse = true;
	
	for(int64_t l = 0; l < ARR_LEN(grammar); l++) {
		if(grammar[l][0] != rule) {
			continue;
		}
		num_nodes = 0;
		successful_rule_parse = true;
		for(int64_t i = 0; i < RULE_LEN - 1; i++) {
			if(grammar[l][i + 1] == &grammar_end) {
				break;
			}

			if(is_grammar_rule(grammar[l][i+1]) == true) {
				// Current grammar token is the left-hand side of
				// another rule. Hence recurse for that rule.
				nodes[i] = parse_rule(tokens, index, grammar[l][i+1]);
				if(nodes[i] == NULL) {
					successful_rule_parse = false;
					for(int64_t l = 0; l < i; l++) {
						free_ast(nodes[i]);
					}
					if(top_node->node.syntax.children != NULL) {
						free(top_node->node.syntax.children);
						top_node->node.syntax.children_count = -1;
						top_node->node.syntax.first_char_index = -1;
						top_node->node.syntax.last_char_index = -1;
					}
					break;
				}
			} else {
				// Current grammar token corresponds to a lexeme. Just
				// parse that lexeme ast an AST subtree and add it.
				nodes[i] = grammar[l][i + 1](tokens, *index);
				if(nodes[i] == NULL) {
					successful_rule_parse = false;
					for(int64_t l = 0; l < i; l++) {
						free_ast(nodes[i]);
					}
					if(top_node->node.syntax.children != NULL) {
						free(top_node->node.syntax.children);
						top_node->node.syntax.children_count = -1;
						top_node->node.syntax.first_char_index = -1;
						top_node->node.syntax.last_char_index = -1;
					}
					break;
				}
				if(nodes[i]->kind != NON_TERMINAL) {
					*index += 1;
				}
			}
			if(successful_rule_parse == true) {
				num_nodes++;
				bool result = ast_add_child(top_node, nodes[i]);
				if(result == false) {
					log_error("Failed to build AST");
					for(int64_t l = 0; l < i; l++) {
						free_ast(nodes[i]);
					}
					free_ast(top_node);
					return NULL;
				}
			}
		}
		if(successful_rule_parse == true) {
			break;
		}
	}
	return top_node;
}

ast* parse_top_level(token_ptr tokens, int64_t index) {
	const int64_t rule_len = 4;
	if(index + rule_len >= tokens.len) {
		return NULL;
	}
	ast* node = parse_rule(tokens, &index, &parse_function_definition);
	if(node == NULL) {
		return NULL;
	}
	return node;
}

void free_ast_helper(ast* syntax_tree) {
	if(syntax_tree == NULL) {
		return;
	}
	if(syntax_tree->kind == NON_TERMINAL) {
		for(int64_t i = 0; i < syntax_tree->node.syntax.children_count; i++) {
			free_ast_helper(syntax_tree->node.syntax.children[i]);
		}
		free(syntax_tree->node.syntax.children);
	}
	free(syntax_tree);
}

void free_ast(ast* syntax_tree) {
	if(syntax_tree == NULL) {
		return;
	}
	free_ast_helper(syntax_tree);
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
		case STATEMENT_LIST:
			str = "STATEMENT_LIST";
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

ast* init_ast_syntax_node(syntax_kind kind, int64_t first, int64_t last) {
	ast* node = malloc(sizeof(*node));
	if(node == NULL) {
		log_error("Failed to allocate memory for abstract syntax tree");
		return NULL;
	}
	node->kind = NON_TERMINAL;
	node->node.syntax = (syntax_construct) {
		.construct = kind,
		.first_char_index = first,
		.last_char_index = last,
		.children = NULL,
		.children_count = -1
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
	node->node.lex_token = lexical_token;
	return node;
}

ast* parse_identifier(token_ptr tokens, int64_t index) {
	if(index >= tokens.len) {
		return NULL;
	}
	if(tokens.ptr[index].token_kind != IDENTIFIER) {
		return NULL;
	}
	ast* node = init_ast_token_node(tokens.ptr[index]);
	if(node == NULL) {
		return NULL;
	}
	return node;
}

ast* parse_close_curly(token_ptr tokens, int64_t index) {
	if(index >= tokens.len) {
		return NULL;
	}
	if(tokens.ptr[index].token_kind != CURLY_CLOSE) {
		return NULL;
	}
	ast* node = init_ast_token_node(tokens.ptr[index]);
	if(node == NULL) {
		return NULL;
	}
	return node;
}

ast* parse_open_curly(token_ptr tokens, int64_t index) {
	if(index >= tokens.len) {
		return NULL;
	}
	if(tokens.ptr[index].token_kind != CURLY_OPEN) {
		return NULL;
	}
	ast* node = init_ast_token_node(tokens.ptr[index]);
	if(node == NULL) {
		return NULL;
	}
	return node;
}

ast* parse_close_paren(token_ptr tokens, int64_t index) {
	if(index >= tokens.len) {
		return NULL;
	}
	if(tokens.ptr[index].token_kind != PAREN_CLOSE) {
		return NULL;
	}
	ast* node = init_ast_token_node(tokens.ptr[index]);
	if(node == NULL) {
		return NULL;
	}
	return node;
}

ast* parse_open_paren(token_ptr tokens, int64_t index) {
	if(index >= tokens.len) {
		return NULL;
	}
	if(tokens.ptr[index].token_kind != PAREN_OPEN) {
		return NULL;
	}
	ast* node = init_ast_token_node(tokens.ptr[index]);
	if(node == NULL) {
		return NULL;
	}
	return node;
}

ast* set_syntax_node(ast** nodes, int64_t num_nodes, syntax_kind kind) {
	ast* syntax_node = init_ast_syntax_node(
		kind,
		nodes[0]->node.syntax.first_char_index,
		nodes[num_nodes - 1]->node.lex_token.last_char_index
	);
	if(syntax_node == NULL) {
		for(int64_t i = 0; i < num_nodes; i++) {
			free_ast(nodes[i]);
		}
		return NULL;
	}
	syntax_node->node.syntax.children = malloc(num_nodes * sizeof(*syntax_node->node.syntax.children));
	if(syntax_node->node.syntax.children == NULL) {
		for(int64_t i = 0; i < num_nodes; i++) {
			free_ast(nodes[i]);
		}
		free_ast(syntax_node);
		log_error("Failed to allocate memory for abstract syntax tree");
		return NULL;
	}
	syntax_node->node.syntax.children_count = num_nodes;
	for(int64_t i = 0; i < num_nodes; i++) {
		syntax_node->node.syntax.children[i] = nodes[i];
	}
	return syntax_node;
	
}

ast* parse_keyword_type(token_ptr tokens, int64_t index) {
	if(index >= tokens.len) {
		return NULL;
	}
	enum Token_Kinds keyword_types[] = {
		KEYWORD_INT,
		KEYWORD_FLOAT
	};
	bool is_keyword_type = false;
	for(int64_t i = 0; i < ARR_LEN(keyword_types); i++) {
		if(tokens.ptr[index].token_kind == keyword_types[i]) {
			is_keyword_type = true;
			break;
		}
	}
	if(is_keyword_type == false) {
		return NULL;
	}
	ast* node = init_ast_token_node(tokens.ptr[index]);
	if(node == NULL) {
		return NULL;
	}
	return node;
}

ast* parse_keyword_return(token_ptr tokens, int64_t index) {
	if(index >= tokens.len) {
		return NULL;
	}
	if(tokens.ptr[index].token_kind != KEYWORD_RETURN) {
		return NULL;
	}
	ast* node = init_ast_token_node(tokens.ptr[index]);
	if(node == NULL) {
		return NULL;
	}
	return node;
}

ast* parse_number(token_ptr tokens, int64_t index) {
	if(index >= tokens.len) {
		return NULL;
	}
	if(tokens.ptr[index].token_kind != NUMBER) {
		return NULL;
	}
	ast* node = init_ast_token_node(tokens.ptr[index]);
	if(node == NULL) {
		return NULL;
	}
	return node;
}

ast* parse_semicolon(token_ptr tokens, int64_t index) {
	if(index >= tokens.len) {
		return NULL;
	}
	if(tokens.ptr[index].token_kind != SEMICOLON) {
		return NULL;
	}
	ast* node = init_ast_token_node(tokens.ptr[index]);
	if(node == NULL) {
		return NULL;
	}
	return node;
}

/* ast* parse_statement(token_ptr tokens, int64_t index) { */
/* 	const int64_t rule_len = 3; */
/* 	if(index + rule_len >= tokens.len) { */
/* 		return NULL; */
/* 	} */
/* 	if(tokens.ptr[index].token_kind != KEYWORD_RETURN || */
/* 	tokens.ptr[index + 1].token_kind != NUMBER || */
/* 	tokens.ptr[index + 2].token_kind != SEMICOLON) { */
/* 		return NULL; */
/* 	} */
/* 	ast* keyword_return_node = init_ast_token_node(tokens.ptr[index]); */
/* 	if(keyword_return_node == NULL) { */
/* 		return NULL; */
/* 	} */
/* 	ast* number_node = init_ast_token_node(tokens.ptr[index + 1]); */
/* 	if(number_node == NULL) { */
/* 		free_ast(keyword_return_node); */
/* 		return NULL; */
/* 	} */
/* 	ast* semicolon_node = init_ast_token_node(tokens.ptr[index + 2]); */
/* 	if(semicolon_node == NULL) { */
/* 		free_ast(keyword_return_node); */
/* 		free_ast(number_node); */
/* 		return NULL; */
/* 	} */
/* 	ast* statement_node = init_ast_syntax_node( */
/* 		STATEMENT, */
/* 		tokens.ptr[index].first_char_index, */
/* 		tokens.ptr[index + 2].last_char_index */
/* 	); */
/* 	if(statement_node == NULL) { */
/* 		free_ast(keyword_return_node); */
/* 		free_ast(number_node); */
/* 		free_ast(semicolon_node); */
/* 		return NULL; */
/* 	} */
/* 	statement_node->children = malloc(rule_len * sizeof(*statement_node->children)); */
/* 	if(statement_node->children == NULL) { */
/* 		free_ast(keyword_return_node); */
/* 		free_ast(number_node); */
/* 		free_ast(semicolon_node); */
/* 		free_ast(statement_node); */
/* 		log_error("Failed to allocate memory for abstract syntax tree"); */
/* 		return NULL; */
/* 	} */
/* 	statement_node->children_count = rule_len; */
/* 	statement_node->children[0] = keyword_return_node; */
/* 	statement_node->children[1] = number_node; */
/* 	statement_node->children[2] = semicolon_node; */
/* 	return statement_node; */
/* } */

ast* parse_type(token_ptr tokens, int64_t index) {
	if(index >= tokens.len) {
		return NULL;
	}
	if(!(tokens.ptr[index].token_kind == KEYWORD_INT || tokens.ptr[index].token_kind == KEYWORD_FLOAT)) {
		// TODO: Somehow make this condition use a table or something
		// which contains all types known in a scope.
		return NULL;
	}
	ast* keyword_int_node = init_ast_token_node(tokens.ptr[index]);
	if(keyword_int_node == NULL) {
		return NULL;
	}
	ast* type_node = init_ast_syntax_node(
		TYPE,
		keyword_int_node->node.lex_token.first_char_index,
		keyword_int_node->node.lex_token.last_char_index
	);
	if(type_node == NULL) {
		free_ast(keyword_int_node);
		return NULL;
	}
	type_node->node.syntax.children = malloc(sizeof(*type_node->node.syntax.children));
	if(type_node->node.syntax.children == NULL) {
		free_ast(keyword_int_node);
		free_ast(type_node);
		log_error("Failed to allocate memory for abstract syntax tree");
		return NULL;
	}
	type_node->node.syntax.children_count = 1;
	type_node->node.syntax.children[0] = keyword_int_node;
	return type_node;
}

ast* get_child(ast* node, int64_t index) {
	if(node == NULL) {
		return NULL;
	}
	if(node->kind == TERMINAL) {
		return NULL;
	}
	if(index < 0 || index >= node->node.syntax.children_count) {
		return NULL;
	}
	return node->node.syntax.children[index];
}

bool is_function_definition(ast* node) {
	if(node == NULL) {
		return false;
	}
	if(node->kind == TERMINAL) {
		return false;
	}
	if(node->node.syntax.construct != FUNCTION_DEFINITION) {
		return false;
	}
	return true;
}

bool is_function_prototype(ast* node) {
	if(node == NULL) {
		return false;
	}
	if(node->kind == TERMINAL) {
		return false;
	}
	if(node->node.syntax.construct != FUNCTION_PROTOTYPE) {
		return false;
	}
	return true;
}

bool is_type(ast* node) {
	if(node == NULL) {
		return false;
	}
	if(node->kind == TERMINAL) {
		return false;
	}
	if(node->node.syntax.construct != TYPE) {
		return false;
	}
	return true;
}

bool is_statement_list(ast* node) {
	if(node == NULL) {
		return false;
	}
	if(node->kind == TERMINAL) {
		return false;
	}
	if(node->node.syntax.construct != STATEMENT_LIST) {
		return false;
	}
	return true;
}

bool is_return_statement(ast* node) {
	if(node == NULL) {
		return false;
	}
	if(node->kind == TERMINAL) {
		return false;
	}
	if(node->node.syntax.construct != RETURN_STATEMENT) {
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

ast* function_statement_list(ast* fun) {
	if(!is_function_definition(fun)) {
		return NULL;
	}
	ast* statement_list = NULL;
	for(int64_t i = 0; get_child(fun, i) != NULL; i++) {
		if(is_statement_list(get_child(fun, i))) {
			statement_list = get_child(fun, i);
			break;
		}
	}
	return statement_list;
}

ast* function_return_statement(ast* fun) {
	if(!is_function_definition(fun)) {
		return NULL;
	}
	ast* statement_list = function_statement_list(fun);
	ast* return_statement = NULL;
	for(int64_t i = 0; get_child(statement_list, i) != NULL; i++) {
		if(is_return_statement(get_child(statement_list, i))) {
			// TODO: There can be several returns. Keep a list of them
			return_statement = get_child(statement_list, i);
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

void print_ast_helper(ast* syntax_tree, int64_t padding_count) {
	for(int64_t i = 0; i < padding_count; i++) {
		printf("  ");
	}
	if(syntax_tree->kind == NON_TERMINAL) {
		printf("%s\n", grammar_token_to_c_str(syntax_tree->node.syntax.construct));
		for(int64_t i = 0; i < syntax_tree->node.syntax.children_count; i++) {
			print_ast_helper(syntax_tree->node.syntax.children[i], padding_count + 1);
		}
	} else {
		printf("%s - %.*s\n", token_to_c_str(syntax_tree->node.lex_token.token_kind), SV_ARG(syntax_tree->node.lex_token.lexeme));
	}
}

void print_ast(ast* syntax_tree) {
	if(syntax_tree == NULL) {
		return;
	}
	print_ast_helper(syntax_tree, 0);
}

 
 
