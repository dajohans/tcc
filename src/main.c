#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "error.h"
#include "file_io.h"
#include "lexer.h"
#include "parser.h"
#include "string.h"



/* LEXER
 * Translates strings into integer tokes. Each lexeme is probably one
 * of the following:
 *   word
 *   number
 *   delimiter
 *   string
 *   comment
 *   header name
 * A word is probably given by the regex
 *     [_a-zA-z][_a-zA-Z0-9]*
 * and number is probably given by the regex
 *     [0-9]+[.eE]?[0-9]
 * but there can also be numbers like .5f which need to be added into
 * this regex. The delimiters include the following:
 *     ( ) { } [ ] + - * % / & | < > = >= <= ; ! ->
 * Note that the delimiters should probably be splir into tokes
 * classes like MATH_OP, PAREN, CURLY_BRACE and so on. Comment is
 * something like
 *     //[*]*
 *     /*[*]**/
/* where [*] means any symbol. * Lastly, header name is something like
 *     <[-_0-9a-zA-Z]+>
 * The priority should probably be to check them in this order:
 *    1. comment
 *    2. string
 *    3. header name
 *    4. number
 *    5. delimiter
 *    6. words
 * Start with word, number, delimiter
 */

int main() {
	
	string file_name = set_string("test-file-3");
	/* string file_name = set_string("test-file"); */
	/* string file_name = set_string("test-file-2"); */
	if(file_name.len == -1) {
		return 1;
	}
	string file_content = read_file(file_name);
	/* string file_content = read_file_c_str("test-file"); */
	if(file_content.cap == -1) {
		free_string(file_name);
		return 1;
	}
	/* printf("Hello, world!\nFile content: '%s'\n", file_content.c_str); */
	int32_ptr token_array = lexer_tokenize_source(file_content);
	/* for(int64_t i = 0; i < token_array.len; i++) { */
	/* 	printf("lexeme: %-10s\n", token_to_c_str(token_array.ptr[i])); */
	
	/* } */

	ast* syntax_tree = parse_function_prototype(token_array, 0);
	print_ast(syntax_tree);
	free_ast(syntax_tree);
	free(token_array.ptr);
	free_string(file_content);
	free_string(file_name);
	return 0;
}
