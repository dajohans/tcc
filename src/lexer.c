#include <ctype.h>
#include <stdlib.h>

#include "error.h"
#include "lexer.h"
#include "pointer.h"

typedef struct keyword_token {
	string name;
	int32_t token;
} lexeme_info;

const lexeme_info keyword_list[] = {
	// TODO add more keywords
	(lexeme_info) {
		.name = (string) { .c_str = "return", .len = C_STR_LEN("return"), .cap = C_STR_LEN("return") + 1 },
		.token = KEYWORD_RETURN
	},
	(lexeme_info) {
		.name = (string) { .c_str = "int", .len = C_STR_LEN("int"), .cap = C_STR_LEN("int") + 1 },
		.token = KEYWORD_INT
	}
};


char* token_to_c_str(int32_t token) {
	char* str = "LEXEME_ERROR";
	switch(token) {
		case IDENTIFIER:
			str = "IDENTIFIER";
			break;
		case KEYWORD_INT:
			str = "KEYWORD_INT";
			break;
		case KEYWORD_RETURN:
			str = "KEYWORD_RETURN";
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

lexeme tokenize_single_char(string source_text, int64_t index, char c, int32_t token) {
	if(!(source_text.c_str[index] == c)) {
		return (lexeme) { .token = LEXEME_ERROR, .first_index = -1, .last_index = -1};
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
		return (lexeme) { .token = LEXEME_ERROR, .first_index = -1, .last_index = -1};
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
	lexeme result = (lexeme) { .token = LEXEME_ERROR, .first_index = -1, .last_index = -1 };
	string tmp = set_string(source_text.c_str + index);
	for(int64_t i = 0; i < ARR_LEN(keyword_list); i++) {
		if(index + keyword_list[i].name.len < source_text.len) {
			char tmp_char = tmp.c_str[keyword_list[i].name.len];
			int64_t tmp_len = tmp.len;
			tmp.c_str[keyword_list[i].name.len] = '\0';
			tmp.len = keyword_list[i].name.len;
			if(str_cmp(keyword_list[i].name, tmp)) {
				result = (lexeme) { .token = keyword_list[i].token, .first_index = index, .last_index = index + keyword_list[i].name.len - 1 };
				break;
			}
			tmp.c_str[keyword_list[i].name.len] = tmp_char;
			tmp.len = tmp_len;
		}
	}
	free_string(tmp);
	return result;
}

lexeme tokenize_id(string source_text, int64_t index) {
	if(!(isalpha(source_text.c_str[index]) || source_text.c_str[index] == '_')) {
		return (lexeme) { .token = LEXEME_ERROR, .first_index = -1, .last_index = -1};
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
	// TODO Perhaps log here that lexical analysis starts
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
		bool identified_token = false;
		if(token_array.len == token_array.cap) {
			token_array = realloc_int32_ptr(token_array);
			if(token_array.len == -1) {
				// Note: The old memory was freed in realloc_int32_ptr
				log_error("Failed to allocate memory for token array during lexing");
				return (int32_ptr){ .ptr = NULL, .len = -1, .cap = -1 };
			}
		}
		// TODO Perhaps make a log entry here: report what remains of
		// the line from index onwards.
		for(int64_t i = 0; i < ARR_LEN(tokenize_fun_ptr); i++) {
			lexeme lex = tokenize_fun_ptr[i](source_text, index);
			if(lex.token != LEXEME_ERROR) {
				identified_token = true;
				token_array.ptr[token_array.len] = lex.token;
				token_array.len++;
				string test = set_string(source_text.c_str + lex.first_index);
				test.c_str[lex.last_index - lex.first_index + 1] = '\0';
				/* printf("lexeme: %-10s\tfirst index %ld last index %ld\n", test.c_str, lex.first_index, lex.last_index); */
				free_string(test);
				index = lex.last_index;
				break;
			}
		}
		// TODO Perhaps make a log entry here: if a token was found,
		// report which substring it was and which token, and if a
		// token was not found then report that
		if(!identified_token) {
			token_array.ptr[token_array.len] = LEXEME_ERROR;
			token_array.len++;
			/* printf("erroneous: %-7c\tat string index %ld\n", source_text.c_str[index], index); */
		}
	}
	return token_array;
}
 
