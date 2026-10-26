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
	if(return_type_node->node.non_terminal.extra.type.id != return_expression_node->node.non_terminal.extra.type.id) {
		return false;
	}
	return true;
}
