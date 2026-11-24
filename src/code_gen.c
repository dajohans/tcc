#include <stdio.h>
#include <stdlib.h>

#include "ast.h"
#include "error.h"
#include "code_gen.h"
#include "parser.h"
#include "string_view.h"

#define ASM_PREAMBLE \
	".intel_syntax noprefix\n" \
	".global _start\n" \
	".text\n" \
	"_start:\n"\
	"call main\n" \
	"mov rdi, rax\n" \
	"mov rax, 60\n" \
	"syscall\n"

arithmetic_tree* ast_to_arithmetic_tree_product_subtree(ast* expr, ast* out_last_expr) {
	if(!is_expression(expr)) {
		return NULL;
	}
	arithmetic_tree* node = malloc(sizeof(*node));
	if(node == NULL) {
		log_error("Failed to allocate node for arithmetic tree");
		return NULL;
	}
	node->node = NULL;
	node->l_child = NULL;
	node->r_child = NULL;
	if(!is_nested_expression(expr)) {
		node->node = expr;
		log_entry(LOG_CODE_GEN, "arithmetic_tree: Creating node: %s", ast_node_kind_to_c_str(node->node));
		*out_last_expr = *expr;
		return node;
	}
	if(is_operator_mult(get_child(expr, 1))) {
		node->node = get_child(expr, 1);
		log_entry(LOG_CODE_GEN, "arithmetic_tree: Creating node: %s", ast_node_kind_to_c_str(node->node));
		node->l_child = ast_to_arithmetic_tree_product_subtree(get_child(expr, 0), out_last_expr);
		log_entry(LOG_CODE_GEN, "arithmetic_tree: Adding operand node: %s", ast_node_kind_to_c_str(node->l_child->node));
		node->r_child = ast_to_arithmetic_tree_product_subtree(get_child(expr, 2), out_last_expr);
		log_entry(LOG_CODE_GEN, "arithmetic_tree: Adding operand node: %s", ast_node_kind_to_c_str(node->r_child->node));
	} else if(is_operator_plus(get_child(expr, 1))) {
		node->node = get_child(expr, 0);
		log_entry(LOG_CODE_GEN, "arithmetic_tree: Creating node: %s", ast_node_kind_to_c_str(node->node));
		*out_last_expr = *node->node;
	}
	return node;
}

arithmetic_tree* ast_to_arithmetic_tree(ast* expr) {
	if(!is_expression(expr)) {
		return NULL;
	}
	arithmetic_tree* node = malloc(sizeof(*node));
	if(node == NULL) {
		log_error("Failed to allocate node for arithmetic tree");
		return NULL;
	}
	node->node = NULL;
	node->l_child = NULL;
	node->r_child = NULL;
	if(!is_nested_expression(expr)) {
		node->node = expr;
		log_entry(LOG_CODE_GEN, "arithmetic_tree: Creating node: %s", ast_node_kind_to_c_str(node->node));
		return node;
	}
	if(is_operator_plus(get_child(expr, 1))) {
		node->node = get_child(expr, 1);
		log_entry(LOG_CODE_GEN, "arithmetic_tree: Creating node: %s", ast_node_kind_to_c_str(node->node));
		node->l_child = ast_to_arithmetic_tree(get_child(expr, 0));
		log_entry(LOG_CODE_GEN, "arithmetic_tree: Adding operand node: %s", ast_node_kind_to_c_str(node->l_child->node));
		node->r_child = ast_to_arithmetic_tree(get_child(expr, 2));
		log_entry(LOG_CODE_GEN, "arithmetic_tree: Adding operand node: %s", ast_node_kind_to_c_str(node->r_child->node));
	} else if (is_operator_mult(get_child(expr, 1))) {
		ast last_product_factor;
		arithmetic_tree* tmp = ast_to_arithmetic_tree_product_subtree(expr, &last_product_factor);
		ast* next_ast_node = get_parent(&last_product_factor);
		// NOTE: next_ast_node should always be a nested expression
		if(is_operator_plus(get_child(next_ast_node, 1))) {
			node->node = get_child(next_ast_node, 1);
			log_entry(LOG_CODE_GEN, "arithmetic_tree: Creating node: %s", ast_node_kind_to_c_str(node->node));
			node->l_child = tmp;
			log_entry(LOG_CODE_GEN, "arithmetic_tree: Adding operand node: %s", ast_node_kind_to_c_str(node->l_child->node));
			node->r_child = ast_to_arithmetic_tree(get_child(next_ast_node, 2));
			log_entry(LOG_CODE_GEN, "arithmetic_tree: Adding operand node: %s", ast_node_kind_to_c_str(node->r_child->node));
		} else if(is_operator_mult(get_child(next_ast_node, 1))) {
			return tmp;
		}
	}
	return node;
}

bool free_arithmetic_tree(arithmetic_tree* at) {
	if(at == NULL) {
		return false;
	}
	bool l_result = false;
	bool r_result = false;
	if(at->l_child != NULL) {
		l_result = free_arithmetic_tree(at->l_child);
	}
	if(at->r_child != NULL) {
		r_result = free_arithmetic_tree(at->r_child);
	}
	free(at);
	return l_result && r_result;
	
}

void print_arithmetic_tree_helper(arithmetic_tree* at, int64_t padding_count) {
	for(int64_t i = 0; i < padding_count; i++) {
		printf("  ");
	}
	if(at->l_child != NULL && at->r_child != NULL) {
		printf("%s\n", token_to_c_str(at->node->node.terminal.token_kind));
		print_arithmetic_tree_helper(at->l_child, padding_count + 1);
		print_arithmetic_tree_helper(at->r_child, padding_count + 1);
	} else {
		printf("%s", grammar_token_to_c_str(at->node->node.non_terminal.kind));
		if(is_int(get_child(at->node, 0))) {
			printf(" - %.*s", SV_ARG(get_child(at->node, 0)->node.terminal.lexeme));
		}
		printf("\n");
	}
}

void print_arithmetic_tree(arithmetic_tree* at) {
	if(at == NULL) {
		return;
	}
	print_arithmetic_tree_helper(at, 0);
}


string add_code_function_label(ast* identifier) {
	if(!is_identifier(identifier)) {
		return uninit_str();
	}
	return str_append_c_str(
		set_string_from_sv(identifier->node.terminal.lexeme),
		":\n"
	);
}

string add_code_expression_helper(ast* expr) {
	if(!is_expression(expr)) {
		return uninit_str();
	}
	string ret_str = uninit_str();
	if(!is_nested_expression(expr)) {
		ast* child = get_child(expr, 0);
		if(child == NULL) {
			return uninit_str();
		}
		ret_str = make_string("mov r9, ");
		ret_str = str_append_sv(ret_str, child->node.terminal.lexeme);
		ret_str = str_append_c_str(ret_str, "\n");
	} else {
		ret_str = add_code_expression_helper(get_child(expr, 2));
		if(invalid_str(ret_str)) {
			return uninit_str();
		}
		ast* child = get_child(expr, 0);
		// child is an expression. Need to get the underlying terminal
		// node, which we denote grandchild.
		ast* grandchild = get_child(child, 0);
		ret_str = str_append_c_str(ret_str, "mov r8, ");
		ret_str = str_append_sv(ret_str, grandchild->node.terminal.lexeme);
		ret_str = str_append_c_str(ret_str, "\n");
		
		child = get_child(expr, 1);
		if(is_operator_plus(child)) {
			ret_str = str_append_c_str(ret_str, "add r9, r8\n");
		} else if(is_operator_mult(child)) {
			ret_str = str_append_c_str(ret_str, "imul r9, r8\n");
		}
	// As long as there are no parenthesis in the arithmetic
	// expression, it should be enough to use 3 registers to compute
	// the expression, perhaps only 2 registers. Suppose these are
	// registers r8, r9, r10. Whenever a parenthesis is encountered,
	// put the the currently used registers on the stack. If only r8
	// is used, put it on the stack; if all are used put all three on
	// the stack. Then proceed to the sub-expression inside the
	// parenthesis. If that sub-expression does not itself contain
	// parentheses, then it can be computed using only r8, r9, r10.
	// When restoring the rergister values from the stack, put the
	// newly computed value from the parenthesis sub-expression in the
	// register of lowest number. For example, if r8 is used, put the
	// computer sub-expression value in r9. If r8 is not used (which
	// could perhaps happen if the first symbol in an expression is a
	// parenthesis), then put the computed sub-expression value in r8.
	// Then proceed.
	// This seems to require a new data structure: stack.
	// With this new stack data structure, we keep track of what has
	// been placed on the memory stack. Since the memory stack is not
	// annotated, we need to keep track of what has been placed there.
	}
	return ret_str;
}

string add_code_expression(ast* expr) {
	if(!is_expression(expr)) {
		return uninit_str();
	}
	arithmetic_tree* at = ast_to_arithmetic_tree(expr);
	print_arithmetic_tree(at);
	free_arithmetic_tree(at);
	
	string ret_str = uninit_str();
	if(!is_nested_expression(expr)) {
		ast* child = get_child(expr, 0);
		if(child == NULL) {
			return uninit_str();
		}
		ret_str = make_string("mov r8, ");
		ret_str = str_append_sv(ret_str, child->node.terminal.lexeme);
		ret_str = str_append_c_str(ret_str, "\n");
	} else {
		ret_str = add_code_expression_helper(expr);
		ret_str = str_append_c_str(ret_str, "mov r8, r9\n");
	}
	return ret_str;
}

string add_code_return_statement(ast* statement) {
	if(!is_return_statement(statement)) {
		return uninit_str();
	}

	string asm_code = add_code_expression(return_statement_expression(statement));
	if(invalid_str(asm_code)) {
		return uninit_str();
	}
	asm_code = str_append_c_str(asm_code, "mov rax, r8\nret\n");
	if(invalid_str(asm_code)) {
		return uninit_str();
	}
	return asm_code;
}

string add_code_function(ast* fun) {
	if(!is_function_definition(fun)) {
		return uninit_str();
	}
	string asm_code = add_code_function_label(function_name(fun));
	ast* scope = function_local_scope(fun);
	if(invalid_str(asm_code) || scope == NULL) {
		return uninit_str();
	}
	ast* statement = NULL;
	string tmp = uninit_str();
	for(int64_t i = 0; i < ast_child_count(scope); i++) {
		statement = get_child(scope, i);
		if(is_return_statement(statement)) {
			tmp = add_code_return_statement(statement);
			if(tmp.cap < 0) {
				free_string(&asm_code);
				return uninit_str();
			}
			asm_code = str_append(
				asm_code,
				tmp
			);
			if(asm_code.cap < 0) {
				return uninit_str();
			}
			free_string(&tmp);
		}
	}
	return asm_code;
}
	
string ast_to_assembly(ast* syntax_tree) {
	// 1. Traverse the tree until we find int main(). Then add a call to main() to the main handling string
	// 2. Find the nodes for each function, add a label for each function to the function implementation string. Also add the assembly code which corresponds to the code block of the function.
	// 3. Add all strings and constant data found in the file to the data string
	/* string main_handling1, impl_handling1, impl_handling2; */
	if(syntax_tree == NULL) {
		return uninit_str();
	}
	if(!is_global_scope(syntax_tree)) {
		return uninit_str();
	}
	string asm_code = make_string(ASM_PREAMBLE);
	if(invalid_str(asm_code)) {
		return uninit_str();
	}
	ast* child = NULL;
	string tmp = uninit_str();
	for(int64_t i = 0; i < ast_child_count(syntax_tree); i++) {
		child = get_child(syntax_tree, i);
		if(!is_function_definition(child)) {
			continue;
		}
		tmp = add_code_function(child);
		if(invalid_str(tmp)) {
			free_string(&asm_code);
			return uninit_str();
		}
		asm_code = str_append(
			asm_code,
			tmp
		);
		free_string(&tmp);
	}
	return asm_code;
}

void code_gen(ast* syntax_tree) {
	FILE *fp = fopen("a.s", "w");
	string asm_code = ast_to_assembly(syntax_tree);
	if(invalid_str(asm_code)) {
		log_error("Failed code generation");
		fclose(fp);
		return;
	}
	fprintf(fp, "%s", asm_code.c_str);
	free_string(&asm_code);
	fclose(fp);
}
