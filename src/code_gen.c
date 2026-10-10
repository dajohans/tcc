#include <stdio.h>
#include <stdlib.h>

#include "code_gen.h"
#include "parser.h"
#include "string.h"

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
		main_handling1 = set_string("xor rdi, rdi\n");
		if(main_handling1.cap < 0) {
			return NULL;
		}
	}
	// TODO: The below implementation assumes that the functions name
	// is main and that the statement is "return 0;". In other words,
	// it's hard coded to be the minimal main program. But this
	// demonstrates the basic idea. It remains to change the
	// implementation to properly max the abstract syntax tree.
	ast* statement_node = syntax_tree->children[2];
	if(syntax_tree->language_construct == FUNCTION_DEFINITION) {
		// NOTE: Assume for now that this is main()
		main_handling1 = set_string(
			"call main\n" \
			"mov rdi, rax\n"
		);
		if(main_handling1.cap < 0) {
			return NULL;
		}
		impl_handling1 = set_string("main:\n");
		if(impl_handling1.cap < 0) {
			free_string(main_handling1);
			return NULL;
		}
		if(statement_node->language_construct == STATEMENT) {
		// NOTE: Assume for now that this is a return statement
			impl_handling2 = set_string(
				"mov rax, 0\n" \
				"ret\n"
			);
			if(impl_handling2.cap < 0) {
				free_string(main_handling1);
				free_string(impl_handling1);
				return NULL;
			}
		}
	}
	string* ret = malloc(2 * sizeof(*ret));
	if(ret == NULL) {
		free_string(main_handling1);
		free_string(impl_handling1);
		free_string(impl_handling2);
		return NULL;
	}
	string impl = concat_string(impl_handling1, impl_handling2);
	if(impl.cap < 0) {
		free_string(main_handling1);
		free_string(impl_handling1);
		free_string(impl_handling2);
		free(ret);
		return NULL;
	}
	free_string(impl_handling1);
	free_string(impl_handling2);
	ret[0] = main_handling1;
	ret[1] = impl;
	return ret;
}

void code_gen(ast* syntax_tree) {
	FILE *fp = fopen("a.s", "w");
	string* impl = ast_to_assembly(syntax_tree);
	if(impl == NULL) {
		fprintf(fp,
			ASSEMBLY_PREAMBLE_1"%s"ASSEMBLY_PREAMBLE_2"%s"ASSEMBLY_PREAMBLE_3"%s",
			"", // data string: Sections .data and .section .rodata and such go here
			"xor rdi, rdi", // main handling string: Code for calling main and handling its return value goes here
			"" // function implementation string: Code which implements all functions goes here, including main()
		);
	} else {
		fprintf(fp,
			ASSEMBLY_PREAMBLE_1"%s"ASSEMBLY_PREAMBLE_2"%s"ASSEMBLY_PREAMBLE_3"%s",
			"", // data string: Sections .data and .section .rodata and such go here
			impl[0].c_str, // main handling string: Code for calling main and handling its return value goes here
			impl[1].c_str // function implementation string: Code which implements all functions goes here, including main()
		);
	}
	fclose(fp);
	free_string(impl[0]);
	free_string(impl[1]);
	free(impl);
}
