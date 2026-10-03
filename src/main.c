#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>

#include "string.h"
#include "error.h"
#include "file_io.h"

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

#define ARR_LEN(array) (sizeof(array) / sizeof(array[0]))

enum Lexer_Tokens {
	ERROR = -1,
	IDENTIFIER,
	KEYWORD,
	OPERATOR,
	PAREN_OPEN,
	PAREN_CLOSE,
	BRACKET_OPEN,
	BRACKET_CLOSE,
	CURLY_OPEN,
	CURLY_CLOSE,
	NUMBER,
	STRING,
	COMMENT,
	COMMA,
	SEMICOLON
};

char* token_to_c_str(int32_t token) {
	char* str = "ERROR";
	switch(token) {
		case IDENTIFIER:
			str = "IDENTIFIER";
			break;
		case KEYWORD:
			str = "KEYWORD";
			break;
		case OPERATOR:
			str = "OPERATOR";
			break;
		case PAREN_OPEN:
			str = "PAREN_OPEN";
			break;
		case PAREN_CLOSE:
			str = "PAREN_CLOSE";
			break;
		case BRACKET_OPEN:
			str = "BRACKET_OPEN";
			break;
		case BRACKET_CLOSE:
			str = "BRACKET_CLOSE";
			break;
		case CURLY_OPEN:
			str = "CURLY_OPEN";
			break;
		case CURLY_CLOSE:
			str = "CURLY_CLOSE";
			break;
		case NUMBER:
			str = "NUMBER";
			break;
		case STRING:
			str = "STRING";
			break;
		case COMMENT:
			str = "COMMENT";
			break;
		case COMMA:
			str = "COMMA";
			break;
		case SEMICOLON:
			str = "SEMICOLON";
			break;
	}
	return str;
}

typedef struct int32_ptr {
	int32_t* ptr;
	int64_t len;
	int64_t cap;
} int32_ptr;

typedef struct lexeme {
	int32_t token;
	int64_t first_index;
	int64_t last_index;
} lexeme;

const string keyword_list[] = {
	// TODO add more keywords
	(string) { .c_str = "return", .len = C_STR_LEN("return"), .cap = C_STR_LEN("return") + 1 }
};


lexeme tokenize_single_char(string source_text, int64_t index, char c, int32_t token) {
	if(!(source_text.c_str[index] == c)) {
		return (lexeme) { .token = ERROR, .first_index = -1, .last_index = -1};
	}
	return (lexeme) { .token = token, .first_index = index, .last_index = index };
}

lexeme tokenize_paren_open(string source_text, int64_t index) {
	return tokenize_single_char(source_text, index, '(', PAREN_OPEN);
}

lexeme tokenize_paren_close(string source_text, int64_t index) {
	return tokenize_single_char(source_text, index, ')', PAREN_CLOSE);
}

lexeme tokenize_curly_open(string source_text, int64_t index) {
	return tokenize_single_char(source_text, index, '{', CURLY_OPEN);
}

lexeme tokenize_curly_close(string source_text, int64_t index) {
	return tokenize_single_char(source_text, index, '}', CURLY_CLOSE);
}

lexeme tokenize_semicolon(string source_text, int64_t index) {
	return tokenize_single_char(source_text, index, ';', SEMICOLON);
}

lexeme tokenize_number(string source_text, int64_t index) {
	// TODO add other number representations, such as scientific
	// notation, floating point numbers and numbers with type suffixes
	// like .5f
	if(!isdigit(source_text.c_str[index])) {
		return (lexeme) { .token = ERROR, .first_index = -1, .last_index = -1};
	}
	int64_t len = 0;
	while(index + len < source_text.len) {
		if(!isdigit(source_text.c_str[index + len])) {
			break;
		} else {
			len++;
		}
	}
	return (lexeme) { .token = NUMBER, .first_index = index, .last_index = index + len - 1 };
}

lexeme tokenize_keyword(string source_text, int64_t index) {
	lexeme result = (lexeme) { .token = ERROR, .first_index = -1, .last_index = -1 };
	string tmp = set_string(source_text.c_str + index);
	for(int64_t i = 0; i < ARR_LEN(keyword_list); i++) {
		if(index + keyword_list[i].len < source_text.len) {
			char tmp_char = tmp.c_str[keyword_list[i].len];
			int64_t tmp_len = tmp.len;
			tmp.c_str[keyword_list[i].len] = '\0';
			tmp.len = keyword_list[i].len;

			if(str_cmp(keyword_list[i], tmp)) {
				result = (lexeme) { .token = KEYWORD, .first_index = index, .last_index = index + keyword_list[i].len - 1 };
				break;
			}
			tmp.c_str[keyword_list[i].len] = tmp_char;
			tmp.len = tmp_len;
		}
	}
	free_string(tmp);
	return result;
}

lexeme tokenize_id(string source_text, int64_t index) {
	if(!(isalpha(source_text.c_str[index]) || source_text.c_str[index] == '_')) {
		return (lexeme) { .token = ERROR, .first_index = -1, .last_index = -1};
	}
	int64_t len = 0;
	while(index + len < source_text.len) {
		if(!(isalnum(source_text.c_str[index + len]) || source_text.c_str[index + len] == '_')) {
			break;
		} else {
			len++;
		}
	}
	// TODO Check if there's an edge case to deal with when index + len == source_text.len
	// For unit tests, make sure to test when both first and last
	// characters of the file are part of an identifier
	return (lexeme) { .token = IDENTIFIER, .first_index = index, .last_index = index + len - 1 };
}

int64_t next_non_whitespace(string source_text, int64_t start_index) {
	if(start_index < 0 || start_index >= source_text.len) {
		return -1;
	}
	int64_t index = 0;
	while(start_index + index < source_text.len) {
		if(!isspace(source_text.c_str[start_index + index])) {
			break;
		} else {
			index++;
		}
	}
	if(start_index + index == source_text.len) {
		return -1;
	}
	return start_index + index;
}

int32_ptr lexer_tokenize_source(string source_text) {
	if(source_text.len < 0) {
		return (int32_ptr){ .ptr = NULL, .len = -1, .cap = -1 };
	} 
	int32_ptr token_array;
	token_array.ptr = malloc(5 * sizeof(*token_array.ptr)); 
	if(token_array.ptr == NULL) {
		log_error("Failed to allocate memory for token array");
		return (int32_ptr){ .ptr = NULL, .len = -1, .cap = -1 };
	}
	token_array.len = 0;
	token_array.cap = 5;
	int64_t token_nr = 0;
	int64_t index = 0;
	while(index < source_text.len) {
		index = next_non_whitespace(source_text, index + 1);
		if(index < 0) {
			break;
		}
		lexeme (*tokenize_fun_ptr[]) (string, int64_t) = {
			&tokenize_keyword,
			&tokenize_id,
			&tokenize_paren_open,
			&tokenize_paren_close,
			&tokenize_curly_open,
			&tokenize_curly_close,
			&tokenize_semicolon,
			&tokenize_number
		};
		for(int64_t i = 0; i < ARR_LEN(tokenize_fun_ptr); i++) {
			lexeme lex = tokenize_fun_ptr[i](source_text, index);
			if(lex.token != ERROR) {
				if(token_array.len == token_array.cap) {
					// TODO Break this out into a function
					int32_ptr tmp;
					tmp.ptr = malloc(2 * token_array.cap * sizeof(*tmp.ptr));
					if(tmp.ptr == NULL) {
						log_error("Failed to allocate memory for token array");
						free(token_array.ptr);
						return (int32_ptr){ .ptr = NULL, .len = -1, .cap = -1 };
					}
					tmp.cap = 2 * token_array.cap;
					tmp.len = token_array.len;
					for(int64_t j = 0; j < token_array.len; j++) {
						tmp.ptr[j] = token_array.ptr[j];
					}
					free(token_array.ptr);
					token_array = tmp;
				}
				token_array.ptr[token_array.len] = lex.token;
				token_array.len++;
				string test = set_string(source_text.c_str + lex.first_index);
				test.c_str[lex.last_index - lex.first_index + 1] = '\0';
				printf("lexeme: %-10s\tfirst index %ld last index %ld\n", test.c_str, lex.first_index, lex.last_index);
				free_string(test);
				index = lex.last_index;
			}
			
		}
	}
	return token_array;
}
 
int main() {
	string file_name = set_string("test-file");
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
	printf("Hello, world!\nFile content: '%s'\n", file_content.c_str);
	int32_ptr token_array = lexer_tokenize_source(file_content);
	for(int64_t i = 0; i < token_array.len; i++) {
		printf("lexeme: %-10s\n", token_to_c_str(token_array.ptr[i]));
	
	}
	free(token_array.ptr);
	free_string(file_content);
	free_string(file_name);
	return 0;
}
