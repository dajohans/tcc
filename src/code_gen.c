#include <stdio.h>
#include <stdlib.h>

#include "error.h"
#include "code_gen.h"
#include "parser.h"
#include "string_view.h"

#define ASSEMBLY_PREAMBLE_1 \
	".intel_syntax noprefix\n" \
	".global _start\n"
	
#define ASSEMBLY_PREAMBLE_2 \
	".text\n" \
	"_start:\n"
	
#define ASSEMBLY_PREAMBLE_3 \
	"mov rax, 60\n" \
	"syscall\n"

string* ast_to_assembly(ast* syntax_tree) {
	// 1. Traverse the tree until we find int main(). Then add a call to main() to the main handling string
	// 2. Find the nodes for each function, add a label for each function to the function implementation string. Also add the assembly code which corresponds to the code block of the function.
	// 3. Add all strings and constant data found in the file to the data string
	string main_handling1, impl_handling1, impl_handling2;
	if(syntax_tree == NULL) {
		return NULL;
	}
	if(!is_global_scope(syntax_tree)) {
		return NULL;
	}
	ast* fun = get_child(syntax_tree, 0);
	
	main_handling1 = str_append_sv(
		make_string("call "),
		function_name(fun)->node.lex_token.lexeme
	);
	main_handling1 = str_append_c_str(
		main_handling1,
		"\n"
		"mov rdi, rax\n"
	);
	if(main_handling1.cap < 0) {
		return NULL;
	}
	impl_handling1 = str_append_c_str(set_string_from_sv(function_name(fun)->node.lex_token.lexeme), ":\n");
	if(impl_handling1.cap < 0) {
		free_string(&main_handling1);
		return NULL;
	}
	// TODO: Now we assume that this is a return statement. That
	// needs to change
	impl_handling2 = str_append_sv(
		make_string("mov rax, "),
		return_statement_expression(function_return_statement(fun))->node.lex_token.lexeme
	);
	impl_handling2 = str_append_c_str(
		impl_handling2,
		"\n"
		"ret\n"
	);
	if(impl_handling2.cap < 0) {
		free_string(&main_handling1);
		free_string(&impl_handling1);
		return NULL;
	}
	string* ret = malloc(2 * sizeof(*ret));
	if(ret == NULL) {
		free_string(&main_handling1);
		free_string(&impl_handling1);
		free_string(&impl_handling2);
		return NULL;
	}
	string impl = concat_string(impl_handling1, impl_handling2);
	if(impl.cap < 0) {
		free_string(&main_handling1);
		free_string(&impl_handling1);
		free_string(&impl_handling2);
		free(ret);
		return NULL;
	}
	free_string(&impl_handling1);
	free_string(&impl_handling2);
	ret[0] = main_handling1;
	ret[1] = impl;
	return ret;
}

void code_gen(ast* syntax_tree) {
	FILE *fp = fopen("a.s", "w");
	string* impl = ast_to_assembly(syntax_tree);
	if(impl == NULL) {
		log_error("Failed code generation");
		/* fprintf(fp, */
		/* 	ASSEMBLY_PREAMBLE_1"%s"ASSEMBLY_PREAMBLE_2"%s"ASSEMBLY_PREAMBLE_3"%s", */
		/* 	"", // data string: Sections .data and .section .rodata and such go here */
		/* 	"xor rdi, rdi", // main handling string: Code for calling main and handling its return value goes here */
		/* 	"" // function implementation string: Code which implements all functions goes here, including main() */
		/* ); */
		fclose(fp);
		return;
	}
	fprintf(fp,
		ASSEMBLY_PREAMBLE_1"%s"ASSEMBLY_PREAMBLE_2"%s"ASSEMBLY_PREAMBLE_3"%s",
		"", // data string: Sections .data and .section .rodata and such go here
		impl[0].c_str, // main handling string: Code for calling main and handling its return value goes here
		impl[1].c_str // function implementation string: Code which implements all functions goes here, including main()
	);
    free_string(&impl[0]);
    free_string(&impl[1]);
    free(impl);
	fclose(fp);
}
