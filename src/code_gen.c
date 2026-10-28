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

string add_code_function_label(ast* identifier) {
	if(!is_identifier(identifier)) {
		return uninit_str();
	}
	return str_append_c_str(
		set_string_from_sv(identifier->node.terminal.lexeme),
		":\n"
	);
}

string add_code_expression(ast* expr) {
	if(!is_expression(expr)) {
		return uninit_str();
	}
	ast* child = get_child(expr, 0);
	if(child == NULL) {
		return uninit_str();
	}
	return set_string_from_sv(child->node.terminal.lexeme);
}

string add_code_return_statement(ast* statement) {
	if(!is_return_statement(statement)) {
		return uninit_str();
	}

	string tmp = add_code_expression(return_statement_expression(statement));
	if(invalid_str(tmp)) {
		return uninit_str();
	}
	string asm_code = str_append(
		make_string("mov rax, "),
		tmp
	);
	free_string(&tmp);
	if(invalid_str(asm_code)) {
		return uninit_str();
	}
	asm_code = str_append_c_str(
		asm_code,
		"\n"
	);
	if(invalid_str(asm_code)) {
		return uninit_str();
	}
	asm_code = str_append_c_str(
		asm_code,
		"ret\n"
	);
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
