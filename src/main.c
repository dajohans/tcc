#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "ast.h"
#include "code_gen.h"
#include "error.h"
#include "file_io.h"
#include "lexer.h"
#include "parser.h"
#include "semantic.h"
#include "string_view.h"

int main() {
	
	/* string file_name = make_string("example_code/failing/minimal_misplaced_identifier.c"); */
	/* string file_name = make_string("example_code/failing/minimal_misplaced_char.c"); */
	/* string file_name = make_string("example_code/failing/minimal_bad_type.c"); */
	/* string file_name = make_string("example_code/failing/minimal_wrong_type.c"); */
	string file_name = make_string("example_code/working/minimal.c");
	if(file_name.len == 0) {
		return 1;
	}
	string file_content = read_file(file_name);
	if(file_content.cap == 0) {
		free_string(&file_name);
		return 1;
	}

	string_view src_sv = (string_view) { .ptr = file_content.c_str, .len = file_content.len };
	token_ptr token_array = lexer_tokenize_source(src_sv);
	/* for(int64_t i = 0; i < token_array.len; i++) { */
	/* 	printf( */
	/* 		"lexeme: %-15s - %.*s - %c - %c\n", */
	/* 		token_to_c_str(token_array.ptr[i].token_kind), */
	/* 		SV_ARG(token_array.ptr[i].lexeme), */
	/* 		file_content.c_str[token_array.ptr[i].first_char_index], */
	/* 		file_content.c_str[token_array.ptr[i].last_char_index] */
	/* 	); */
	/* } */

	ast* syntax_tree = parse_top_level(token_array, 0);
	
	/* print_ast(syntax_tree); */
	print_type_table(syntax_tree->node.non_terminal.extra.type_table);
	if(!parser_validate_types(syntax_tree)) {
		log_error("Invalid type");
	}
	print_ast(syntax_tree);

	if(!parser_assign_expression_types(syntax_tree)) {
		log_error("Failed to assign type to expression");
	}
	
	print_expression_types(syntax_tree);
	
	if(!analyze_semantics(syntax_tree)) {
		log_error("Type mismatch - Return value type and function return type differ");
	}
	code_gen(syntax_tree);
	
	free_ast(syntax_tree);
	free(token_array.ptr);
	free_string(&file_content);
	free_string(&file_name);
	return 0;
}
