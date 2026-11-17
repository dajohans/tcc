#include <stdio.h>
#include <stdlib.h>

#include "ast.h"
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
ast* parse_local_scope(token_ptr, int64_t) { return NULL; }
ast* parse_global_scope(token_ptr, int64_t) { return NULL; }
ast* parse_type(token_ptr, int64_t) { return NULL; }
ast* parse_expression_r(token_ptr, int64_t) { return NULL; }
ast* parse_expression(token_ptr, int64_t) { return NULL; }


/* a grammar rule with several alternatives such as
 * 'type -> keyword_int OR keyword_float'
 * is represented by two arrays; one array for each alternative. So we
 * would have one array for 'type -> keyword_int' and one for
 * 'type -> keyword_float'.
 *
 * A grammar rule with arbitrary amount of repetition such as 
 * 'local_scope -> statement*'
 * is represented by a grammar rule which ends with the function
 * grammar_repeat.
 */

#define RULE_LEN 10
ast* (*grammar[][RULE_LEN]) (token_ptr, int64_t) = {
	{ parse_global_scope,        parse_function_definition,                                                        grammar_end, grammar_end, grammar_end, grammar_end, grammar_end, grammar_end, grammar_end, grammar_end },
	{ parse_type,                parse_type_identifier,                                                            grammar_end, grammar_end, grammar_end, grammar_end, grammar_end, grammar_end, grammar_end, grammar_end },
	{ parse_expression,          parse_expression, parse_operator_plus, parse_expression,                          grammar_end, grammar_end, grammar_end, grammar_end, grammar_end, grammar_end },
	{ parse_expression,          parse_number,                                                                     grammar_end, grammar_end, grammar_end, grammar_end, grammar_end, grammar_end, grammar_end, grammar_end },
	{ parse_return_statement,    parse_keyword_return, parse_expression, parse_semicolon,                          grammar_end, grammar_end, grammar_end, grammar_end, grammar_end, grammar_end },
	{ parse_function_prototype,  parse_type, parse_identifier, parse_open_paren, parse_close_paren,                grammar_end, grammar_end, grammar_end, grammar_end, grammar_end  },
	{ parse_function_definition, parse_function_prototype, parse_open_curly, parse_local_scope, parse_close_curly, grammar_end, grammar_end, grammar_end, grammar_end, grammar_end  },
	{ parse_local_scope,         parse_return_statement,                                                           grammar_end, grammar_end, grammar_end, grammar_end, grammar_end, grammar_end, grammar_end, grammar_end  }
};

syntax_kind rule_kind[] = {
	GLOBAL_SCOPE,
	TYPE,
	EXPRESSION,
	EXPRESSION,
	RETURN_STATEMENT,
	FUNCTION_PROTOTYPE,
	FUNCTION_DEFINITION,
	LOCAL_SCOPE
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

bool is_scope_rule(syntax_kind rule) {
	return rule == GLOBAL_SCOPE || rule == LOCAL_SCOPE;
}

syntax_kind get_rule_kind(ast* (rule) (token_ptr, int64_t)) {
	syntax_kind current_rule_kind = AST_ERROR;
	for(int64_t i = 0; i < ARR_LEN(grammar); i++) {
		if(rule == grammar[i][0]) {
			current_rule_kind = rule_kind[i];
			break;
		}
	}
	return current_rule_kind;
}

bool is_left_recursive(int64_t rule_index) {
	if(rule_index < 0 || rule_index >= ARR_LEN(grammar)) {
	return false;
	}
	if(grammar[rule_index][0] != grammar[rule_index][1]) {
		return false;
	}
	return true;
}

ast* parse_rule_non_recursive(token_ptr tokens, int64_t index, ast* (rule) (token_ptr, int64_t)) {
/* Go through the grammar rules in the 'grammar' variable in order.
 * Attempt to parse the rules of the same kind as 'rule', but only those
 * that are not left-recursive, untill one parses successfully.
 */
	if(index >= tokens.len) {
		return NULL;
	}
	ast_ptr node_ptr = alloc_ast_ptr(RULE_LEN - 1); // -1 since the first element in each grammar rule is a placeholder
	if(!ast_ptr_is_valid(node_ptr)) {
		return NULL;
	}
	bool successful_rule_parse = true;
	int64_t index_progress = 0;
	syntax_kind current_rule_kind = get_rule_kind(rule);
	for(int64_t i = 0; i < ARR_LEN(grammar); i++) {
		if(current_rule_kind != rule_kind[i]) {
			continue;
		}
		if(is_left_recursive(i)) {
			continue;
		}
	    successful_rule_parse = true;
		for(int64_t l = 0; l < RULE_LEN - 1; l++) {
			if(grammar[i][l + 1] == &grammar_end) {
				break;
			}
			if(is_grammar_rule(grammar[i][l + 1])) {
				node_ptr.ptr[l] = parse_rule_kind(tokens, index + index_progress, grammar[i][l + 1]);
			
			} else {
				node_ptr.ptr[l] = grammar[i][l + 1](tokens, index + index_progress);
			}
			if(node_ptr.ptr[l] == NULL) {
				printf("failed to match non-rec; reverting %ld matches\n", l);
				successful_rule_parse = false;
				break;
			}
			printf("matched non-rec\n");
			print_ast(node_ptr.ptr[l]);
			node_ptr.len += 1;
			index_progress += ast_num_tokens(node_ptr.ptr[l]);
		}
		if(successful_rule_parse) {
			break;
		}
	}
	if(!successful_rule_parse) {
		for(int64_t l = 0; l < node_ptr.len; l++) {
			free_ast(node_ptr.ptr[l]);
		}
		free(node_ptr.ptr);
		node_ptr = ast_ptr_default();
		return NULL;
	}
	
	ast* matched_node = init_ast_syntax_node(current_rule_kind, -1, -1);
	if(matched_node == NULL) {
		log_error("Failed to allocate memory for ast top node");
		for(int64_t l = 0; l < node_ptr.len; l++) {
			free_ast(node_ptr.ptr[l]);
		}
		free(node_ptr.ptr);
		return NULL;
	}
	if(is_scope_rule(current_rule_kind)) {
		type_table tbl = create_global_type_table();
		if(tbl.table == NULL) {
			log_error("Failed to add type table to global scope");
			for(int64_t l = 0; l < node_ptr.len; l++) {
				free_ast(node_ptr.ptr[l]);
			}
			free(node_ptr.ptr);
			free_ast(matched_node);
			return NULL;
		}
		matched_node->node.non_terminal.extra.type_table = tbl;
	}
	for(int64_t i = 0; i < node_ptr.len; i++) {
		bool result = ast_add_child(matched_node, node_ptr.ptr[i]);
		if(result == false) {
			log_error("Failed to build AST");
			for(int64_t l = 0; l < i; l++) {
				free_ast(node_ptr.ptr[i]);
			}
			free(node_ptr.ptr);
			free_ast(matched_node);
			return NULL;
		}
	}
	free(node_ptr.ptr);
	return matched_node;
}

ast* parse_current_rule(token_ptr tokens, int64_t index, int64_t rule_index) {
/* Attempt to parse the grammar rule described by the array
 * grammar[rule_index].
 */
	if(index >= tokens.len) {
		return NULL;
	}
	ast_ptr node_ptr = alloc_ast_ptr(RULE_LEN - 1); // -1 since the first element in each grammar rule is a placeholder
	if(!ast_ptr_is_valid(node_ptr)) {
		return NULL;
	}
	bool successful_rule_parse = true;
	int64_t index_progress = 0;
	syntax_kind current_rule_kind = rule_kind[rule_index];
	int64_t start_index = 0;
	if(is_left_recursive(rule_index)) {
		node_ptr.ptr[0] = parse_rule_non_recursive(tokens, index, grammar[rule_index][0]);
		if(node_ptr.ptr[0] == NULL) {
			free(node_ptr.ptr);
			return NULL;
		}
		node_ptr.len += 1;
		printf("matched\n");
		print_ast(node_ptr.ptr[0]);
		start_index += 1;
		index_progress += ast_num_tokens(node_ptr.ptr[0]);
	}
	for(int64_t l = start_index; l < RULE_LEN - 1; l++) {
		if(grammar[rule_index][l + 1] == &grammar_end) {
			break;
		}
		if(is_grammar_rule(grammar[rule_index][l + 1])) {
			node_ptr.ptr[l] = parse_rule_kind(tokens, index + index_progress, grammar[rule_index][l + 1]);
		
		} else {
			node_ptr.ptr[l] = grammar[rule_index][l + 1](tokens, index + index_progress);
		}
		if(node_ptr.ptr[l] == NULL) {
			printf("failed to match; reverting %ld matches\n", l);
			successful_rule_parse = false;
			break;
		}
		printf("matched\n");
		print_ast(node_ptr.ptr[l]);
		node_ptr.len += 1;
		index_progress += ast_num_tokens(node_ptr.ptr[l]);
	}
	if(!successful_rule_parse) {
		for(int64_t l = 0; l < node_ptr.len; l++) {
			free_ast(node_ptr.ptr[l]);
		}
		free(node_ptr.ptr);
		node_ptr = ast_ptr_default();
		return NULL;
	}
	
	ast* matched_node = init_ast_syntax_node(current_rule_kind, -1, -1);
	if(matched_node == NULL) {
		log_error("Failed to allocate memory for ast top node");
		for(int64_t l = 0; l < node_ptr.len; l++) {
			free_ast(node_ptr.ptr[l]);
		}
		free(node_ptr.ptr);
		return NULL;
	}
	if(is_scope_rule(current_rule_kind)) {
		type_table tbl = create_global_type_table();
		if(tbl.table == NULL) {
			log_error("Failed to add type table to global scope");
			for(int64_t l = 0; l < node_ptr.len; l++) {
				free_ast(node_ptr.ptr[l]);
			}
			free(node_ptr.ptr);
			free_ast(matched_node);
			return NULL;
		}
		matched_node->node.non_terminal.extra.type_table = tbl;
	}
	for(int64_t i = 0; i < node_ptr.len; i++) {
		bool result = ast_add_child(matched_node, node_ptr.ptr[i]);
		if(result == false) {
			log_error("Failed to build AST");
			for(int64_t l = 0; l < i; l++) {
				free_ast(node_ptr.ptr[i]);
			}
			free(node_ptr.ptr);
			free_ast(matched_node);
			return NULL;
		}
	}
	free(node_ptr.ptr);
	return matched_node;
}

ast* parse_rule_kind(token_ptr tokens, int64_t index, ast* (rule) (token_ptr, int64_t)) {
/* Go through the grammar rules in the 'grammar' variable in order.
 * Attempt to parse the rules of the same kind as 'rule', untill one
 * parses successfully.
 */
	if(index >= tokens.len) {
		return NULL;
	}
	syntax_kind rule_kind = get_rule_kind(rule);
	if(rule_kind == AST_ERROR) {
		return NULL;
	}
	
	ast* top_node = NULL;
	for(int64_t l = 0; l < ARR_LEN(grammar); l++) {
		if(grammar[l][0] != rule) {
			continue;
		}
		top_node = parse_current_rule(tokens, index, l);
		if(top_node != NULL) {
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
	ast* node = parse_rule_kind(tokens, index, &parse_global_scope);
	if(node == NULL) {
		return NULL;
	}
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

ast* parse_operator_plus(token_ptr tokens, int64_t index) {
	if(index >= tokens.len) {
		return NULL;
	}
	if(tokens.ptr[index].token_kind != OPERATOR_PLUS) {
		return NULL;
	}
	ast* node = init_ast_token_node(tokens.ptr[index]);
	if(node == NULL) {
		return NULL;
	}
	return node;
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

ast* parse_type_identifier(token_ptr tokens, int64_t index) {
	if(index >= tokens.len) {
		return NULL;
	}
	if(!token_is_id_or_type(tokens.ptr[index])) {
		return NULL;
	}
	ast* node = init_ast_token_node(tokens.ptr[index]);
	if(node == NULL) {
		return NULL;
	}
	return node;
}

bool is_type_in_table(type_table types, ast* type_node) {
	if(!is_type(type_node)) {
		return false;
	}
	if(types.table == NULL) {
		return false;
	}
	ast* type_id = get_child(type_node, 0);
	bool found_type = false;
	for(int64_t i = 0; i < types.table_len; i++) {
		if(sv_cmp(type_id->node.terminal.lexeme, types.table[i].name)) {
			found_type = true;
			break;
		}
	}
	return found_type;
}

bool type_exists(ast* type_node) {
	if(!is_type(type_node)) {
		return false;
	}
	bool found_type = false;
	for(ast* parent = get_parent(type_node); parent != NULL; parent = get_parent(parent)) {
		if(!is_scope(parent)) {
			continue;
		}
		if(is_type_in_table(parent->node.non_terminal.extra.type_table, type_node)) {
			found_type = true;
			break;
		}
	}
	return found_type;
}

type_info find_type_in_table(type_table types, ast* type_node) {
	if(!is_type(type_node)) {
		return (type_info) {
			.id = -1,
			.name = (string_view) {
				.ptr = NULL,
				.len = -1
			}
		};
	}
	if(types.table == NULL) {
		return (type_info) {
			.id = -1,
			.name = (string_view) {
				.ptr = NULL,
				.len = -1
			}
		};
	}
	ast* type_id = get_child(type_node, 0);
	type_info found_type = (type_info) {
		.id = -1,
		.name = (string_view) {
			.ptr = NULL,
			.len = -1
		}
	};
	for(int64_t i = 0; i < types.table_len; i++) {
		if(sv_cmp(type_id->node.terminal.lexeme, types.table[i].name)) {
			found_type = types.table[i];
			break;
		}
	}
	return found_type;
}

type_info find_type(ast* type_node) {
	if(!is_type(type_node)) {
		return (type_info) {
			.id = -1,
			.name = (string_view) {
				.ptr = NULL,
				.len = -1
			}
		};
	}
	type_info found_type = (type_info) {
		.id = -1,
		.name = (string_view) {
			.ptr = NULL,
			.len = -1
		}
	};
	type_info tmp_type = (type_info) {
		.id = -1,
		.name = (string_view) {
			.ptr = NULL,
			.len = -1
		}
	};
	for(ast* parent = get_parent(type_node); parent != NULL; parent = get_parent(parent)) {
		if(!is_scope(parent)) {
			continue;
		}
		tmp_type = find_type_in_table(parent->node.non_terminal.extra.type_table, type_node);
		if(tmp_type.id != -1) {
			// TODO: Valid type IDs are >= 1. Define constant
			// somewhere to avoid magic constant, and maybe implement
			// a function which checks if the type is valid.
			found_type = tmp_type;
			break;
		}
	}
	return found_type;
}

bool parser_validate_types(ast* syntax_tree) {
	// This function should go through the ast and do two things:
	//   1. Add any typedefs and structs to the type table 
	//   2. Check that any ast node marked as type has a lexeme which
	//      exists in an available type table.
	// TODO: Still need to implement point 1 above.
	if(syntax_tree == NULL) {
		return false;
	}
	if(!ast_can_have_children(syntax_tree)) {
		return true;
	}
	bool all_types_valid = true;
	type_info tmp_type = (type_info) {
		.id = -1,
		.name = (string_view) {
			.ptr = NULL,
			.len = -1
		}
	};
	for(int64_t i = 0; i < ast_child_count(syntax_tree); i++) {
		ast* child = get_child(syntax_tree, i);
		if(!is_type(child)) {
			if(!parser_validate_types(child)) {
				all_types_valid = false;
			}
			continue;
		}
		tmp_type = find_type(child);
		if(tmp_type.id == -1) {
			child->node.non_terminal.kind = INVALID_TYPE;
			child->node.non_terminal.extra.type = (type_info) {
				.id = -1,
				.name = (string_view) {
					.ptr = NULL,
					.len = -1
				}
			};
			all_types_valid = false;
			continue;
		}
		child->node.non_terminal.extra.type = tmp_type;
	}
	return all_types_valid;
}

bool determine_expression_type(ast* expr) {
	if(expr == NULL) {
		return false;
	}
	if(expr->node.non_terminal.extra.type.name.ptr != NULL) {
		return true;
	}
	ast* child = get_child(expr, 0);
	bool determined_type = false;
	type_info expr_type = (type_info) {
		.id = -1,
		.name = set_sv_ptr("invalid type", c_str_len("invalid type"))
	};
	if(is_int(child)) {
		char* type_str = "int";
		for(ast* parent = get_parent(expr); parent != NULL; parent = get_parent(parent)) {
			if(!is_scope(parent)) {
				continue;
			}
			type_table types = parent->node.non_terminal.extra.type_table;
			
			for(int64_t i = 0; i < types.table_len; i++) {
				string_view type_sv = set_sv_ptr(type_str, c_str_len(type_str));
				if(sv_cmp(types.table[i].name, type_sv)) {
					expr_type = types.table[i];
					determined_type = true;
					break;
				}
			}
			if(determined_type) {
				break;
			}
		}
	}
	expr->node.non_terminal.extra.type = expr_type;
	return determined_type;
}

bool parser_assign_expression_types(ast* syntax_tree) {
	if(syntax_tree == NULL) {
		return false;
	}
	if(!ast_can_have_children(syntax_tree)) {
		return true;
	}
	bool all_expression_types_valid = true;
	for(int64_t i = 0; i < ast_child_count(syntax_tree); i++) {
		ast* child = get_child(syntax_tree, i);
		if(!is_expression(child)) {
			if(!parser_assign_expression_types(child)) {
				all_expression_types_valid = false;
			}
			continue;
		}
		if(is_nested_expression(child)) {
			if(!parser_assign_expression_types(child)) {
				all_expression_types_valid = false;
				continue;
			}
			ast* expr_1 = get_child(child, 0);
			ast* expr_2 = get_child(child, 2);
			
			bool expr_1_type_determined = determine_expression_type(expr_1);
			bool expr_2_type_determined = determine_expression_type(expr_2);
			if(!(expr_1_type_determined && expr_2_type_determined)) {
				all_expression_types_valid = false;
				continue;
			}
			if(expr_1->node.non_terminal.extra.type.id != expr_2->node.non_terminal.extra.type.id) {
				// TODO: Adapt this to handle automatic conversion.
				// For example an int multiplied by a float becomes an
				// int or a float even if they are not the same type.
				all_expression_types_valid = false;
				continue;
			}
			child->node.non_terminal.extra.type = expr_1->node.non_terminal.extra.type;
			continue;
		}
		if(!determine_expression_type(child)) {
			all_expression_types_valid = false;
		}
	}
	return all_expression_types_valid;
}
