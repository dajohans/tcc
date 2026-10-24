#include <stdio.h>

#include "ast.h"
#include "error.h"
#include "lexer.h"
#include "semantic.h"
#include "string_view.h"

bool analyze_semantics(ast* syntax_tree) {
	if(!is_global_scope(syntax_tree)) {
		return false;
	}
	bool result = true;
	for(int64_t i = 0; get_child(syntax_tree, i) != NULL; i++) {
		if(is_function_definition(get_child(syntax_tree, i))) {
			result = return_type_check(get_child(syntax_tree, i));
		}
	}
	return result;
}

bool return_type_check(ast* fun) {
	ast* return_type_node = function_return_type(fun);
	ast* return_expression_node = return_statement_expression(function_return_statement(fun));
	if(return_type_node == NULL || return_expression_node == NULL) {
		return false;
	}
	string int_str = make_string("int");
	if(int_str.cap < 0) {
		return false;
	}
	string float_str = make_string("float");
	if(float_str.cap < 0) {
		free_string(&int_str);
		return false;
	}
	string_view int_sv = set_sv_ptr(int_str.c_str, int_str.len);
	string_view float_sv = set_sv_ptr(float_str.c_str, float_str.len);
	type_info type_table[] = {
		(type_info) { .id = KEYWORD_INT, .name = int_sv },
		(type_info) { .id = KEYWORD_FLOAT, .name = float_sv }
	};
	type_info function_return_type = (type_info) {
		.id = -1,
		.name = { .ptr = NULL, .len = -1 }
	};
	for(int64_t i = 0; i < ARR_LEN(type_table); i++) {
		if(get_child(return_type_node, 0)->node.terminal.token_kind == type_table[i].id) {
			function_return_type = type_table[i];
		}
	}
	if(function_return_type.id == -1) {
		free_string(&int_str);
		free_string(&float_str);
		log_error("Failed return value check - invalid return type in function prototype");
		return false;
	}
	type_info return_value_type = (type_info) {
		.id = -1,
		.name = { .ptr = NULL, .len = -1 }
	};
	// TODO: This is still hard coded for 'return 0;'
	if(return_expression_node->node.terminal.token_kind == NUMBER) {
		return_value_type = type_table[0];
	}
	if(return_value_type.id == -1) {
		free_string(&int_str);
		free_string(&float_str);
		log_error("Failed return value check - invalid type of returned expression");
		return false;
	}
	if(return_value_type.id != function_return_type.id) {
		free_string(&int_str);
		free_string(&float_str);
		return false;
	}
	free_string(&int_str);
	free_string(&float_str);
	return true;
}
