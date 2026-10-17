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

 /* TODO: Instead of defining each grammar rule with its own C
  * function, define the grammar rules in an array using something
  * like EBNF notation for context free grammars:
  *     int [][] = {
  *         { TYPE, KEYWORD_INT },
  *         { STATEMENT, KEYWORD_RETURN, NUMBER, SEMICOLON },
  *         { STATEMENT_LIST, EBNF_REPEAT_BEGIN, STATEMENT, EBNF_REPEAT_END },
  *         ...
  *         ...
  *     }
  * The idea with here is that the leftmost value is the name of a
  * grammar rule, and then comes the right-hand side of that rule. By
  * defining special constants like EBNF_REPEAT_BEGIN,
  * EBNF_REPEAT_END, EBNF_SEPARATOR then we could get functionality
  * like [statement] meaning repetition of statemet and
  * keyword_int | keyword_float meaning either an int or a float. So
  * EBNF_REPEAT_BEGIN is [, EBNF_REPEAT_END is] and EBNF_SEPARATOR is
  * |. While it is awkward to write EBNF with words like this, it's
  * better than implementing separate functions for each grammar rule
  * and it makes parser generation a little bit like when using YACC
  * or Bison but without actually using YACC or Bison.
  */
/* void grammar_end(token_ptr, int64_t) { */
/* } */
/* void grammar_repeat(token_ptr, int64_t) { */
/* } */
/* ast* (*fun_ptr[][7]) (token_ptr, int64_t) = { */
/* 	{ parse_keyword_int, parse_keyword_float, parse_from_type_table, grammar_end, grammar_end, grammar_end, grammar_end }, */
/* 	{ parse_type, parse_identifier, parse_open_paren, parse_close_paren, grammar_end, grammar_end, grammar_end }, */
/* 	{ parse_expression, parse_arithmetic_op, parse_expression, grammar_end, grammar_end, grammar_end, grammar_end }, */
/* 	{ parse_keyword_return, parse_expression, parse_semicolon, grammar_end, grammar_end, grammar_end, grammar_end }, */
/* 	{ parse_if_statement, parse_open_paren, parse_expression, parse_close_paren, parse_open_curly, parse_statement_list, parse_close_curly}, */
/* 	{ parse_statement, grammar_repeat, grammar_end, grammar_end, grammar_end, grammar_end, grammar_end }, // Repeat the thing before parse_statement */
/* }; */

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

/* syntax_kind grammar_rule[] = { */
/* 	TYPE, */
/* 	FUNCTION_PROTOTYPE, */
/* 	EXPRESSION, */
/* 	STATEMENT, */
/* 	STATEMENT, */
/* 	STATEMENT_LIST */
/* }; */

#define RULE_LEN 10
int64_t grammar_rules [][RULE_LEN] = {

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
		case TYPE:
			str = "TYPE";
			break;
		case STATEMENT:
			str = "STATEMENT";
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
		.last_char_index = last
	};
	node->children = NULL;
	node->children_count = -1;
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
	node->children = NULL;
	node->children_count = -1;
	return node;
}

ast* parse_function_definition(token_ptr tokens, int64_t index) {
	const int64_t rule_len = 4;
	if(index + rule_len >= tokens.len) {
		return NULL;
	}
	ast* function_prototype_node = parse_function_prototype(tokens, index);
	if(function_prototype_node == NULL) {
		return NULL;
	}
	ast* statement_node = parse_statement(tokens, index + function_prototype_node->children_count + 1);
	if(statement_node == NULL) {
		free_ast(function_prototype_node);
		return NULL;
	}
	if(tokens.ptr[index + function_prototype_node->children_count].token_kind != CURLY_OPEN ||
	tokens.ptr[index + function_prototype_node->children_count + 1 + statement_node->children_count].token_kind != CURLY_CLOSE) {
		free_ast(function_prototype_node);
		free_ast(statement_node);
		return NULL;
	}
	ast* curly_open_node = init_ast_token_node(tokens.ptr[index + function_prototype_node->children_count]);
	if(curly_open_node == NULL) {
		free_ast(function_prototype_node);
		free_ast(statement_node);
		return NULL;
	}
	ast* curly_close_node = init_ast_token_node(tokens.ptr[index + function_prototype_node->children_count + 1 + statement_node->children_count]);
	if(curly_close_node == NULL) {
		free_ast(function_prototype_node);
		free_ast(curly_open_node);
		free_ast(statement_node);
		return NULL;
	}
	ast* function_definition_node = init_ast_syntax_node(
		FUNCTION_DEFINITION,
		function_prototype_node->node.syntax.first_char_index,
		curly_close_node->node.lex_token.last_char_index
	);
	if(function_definition_node == NULL) {
		free_ast(function_prototype_node);
		free_ast(curly_open_node);
		free_ast(statement_node);
		free_ast(curly_close_node);
		return NULL;
	}
	function_definition_node->children = malloc(rule_len * sizeof(*function_prototype_node->children));
	if(function_definition_node->children == NULL) {
		free_ast(function_prototype_node);
		free_ast(curly_open_node);
		free_ast(statement_node);
		free_ast(curly_close_node);
		free_ast(function_definition_node);
		log_error("Failed to allocate memory for abstract syntax tree");
		return NULL;
	}
	function_definition_node->children_count = rule_len;
	function_definition_node->children[0] = function_prototype_node;
	function_definition_node->children[1] = curly_open_node;
	function_definition_node->children[2] = statement_node;
	function_definition_node->children[3] = curly_close_node;
	return function_definition_node;
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
	syntax_node->children = malloc(num_nodes * sizeof(*syntax_node->children));
	if(syntax_node->children == NULL) {
		for(int64_t i = 0; i < num_nodes; i++) {
			free_ast(nodes[i]);
		}
		free_ast(syntax_node);
		log_error("Failed to allocate memory for abstract syntax tree");
		return NULL;
	}
	syntax_node->children_count = num_nodes;
	for(int64_t i = 0; i < num_nodes; i++) {
		syntax_node->children[i] = nodes[i];
	}
	return syntax_node;
	
}

ast* parse_function_prototype(token_ptr tokens, int64_t index) {
	const int64_t rule_len = 4;
	if(index + rule_len >= tokens.len) {
		return NULL;
	}
	
	ast *nodes[rule_len];
	ast* (*parse_fun_ptr[]) (token_ptr, int64_t) = {
		parse_type,
		parse_identifier,
		parse_open_paren,
		parse_close_paren
	};
	
	for(int64_t i = 0; i < ARR_LEN(nodes); i++) {
		nodes[i] = parse_fun_ptr[i](tokens, index);
		if(nodes[i] == NULL) {
			for(int64_t l = 0; l < i; l++) {
				free_ast(nodes[i]);
			}
			return NULL;
		}
		if(nodes[i]->kind == NON_TERMINAL) {
			// TODO: This needs to be the number of tokens in the ast
			// nodes[i]. Even a type node can consist of two tokens,
			// e.g. for a 'struct node'.
			index += nodes[i]->children_count;
		} else {
			index += 1;
		}
	}
	
	ast* function_prototype_node = set_syntax_node(nodes, ARR_LEN(nodes), FUNCTION_PROTOTYPE);
	if(function_prototype_node == NULL) {
		return NULL;
	}
	return function_prototype_node;
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

ast* parse_statement(token_ptr tokens, int64_t index) {
	const int64_t rule_len = 3;
	if(index + rule_len >= tokens.len) {
		return NULL;
	}
	
	ast *nodes[rule_len];
	ast* (*parse_fun_ptr[]) (token_ptr, int64_t) = {
		parse_keyword_return,
		parse_number,
		parse_semicolon
	};
	
	for(int64_t i = 0; i < ARR_LEN(nodes); i++) {
		nodes[i] = parse_fun_ptr[i](tokens, index);
		if(nodes[i] == NULL) {
			for(int64_t l = 0; l < i; l++) {
				free_ast(nodes[i]);
			}
			return NULL;
		}
		if(nodes[i]->kind == NON_TERMINAL) {
			// TODO: This needs to be the number of tokens in the ast
			// nodes[i]. Even a type node can consist of two tokens,
			// e.g. for a 'struct node'.
			index += nodes[i]->children_count;
		} else {
			index += 1;
		}
	}
	
	ast* statement_node = set_syntax_node(nodes, ARR_LEN(nodes), STATEMENT);
	if(statement_node == NULL) {
		return NULL;
	}
	return statement_node;
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
	type_node->children = malloc(sizeof(*type_node->children));
	if(type_node->children == NULL) {
		free_ast(keyword_int_node);
		free_ast(type_node);
		log_error("Failed to allocate memory for abstract syntax tree");
		return NULL;
	}
	type_node->children_count = 1;
	type_node->children[0] = keyword_int_node;
	return type_node;
}

void print_ast_helper(ast* syntax_tree, int64_t padding_count) {
	for(int64_t i = 0; i < padding_count; i++) {
		printf("  ");
	}
	if(syntax_tree->kind == NON_TERMINAL) {
		printf("%s\n", grammer_token_to_c_str(syntax_tree->node.syntax.construct));
	} else {
		printf("%s - %.*s\n", token_to_c_str(syntax_tree->node.lex_token.token_kind), SV_ARG(syntax_tree->node.lex_token.lexeme));
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

 
 
