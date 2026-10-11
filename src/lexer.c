#include <ctype.h>
#include <stdlib.h>

#include "error.h"
#include "lexer.h"
#include "pointer.h"

typedef struct keyword_token {
	string lexeme;
	int32_t token_kind;
	// TODO: Change name so to token_kind instead of token
} keyword_token;

// TODO: Change name, maybe keyword_token_list
const keyword_token keyword_token_list[] = {
	// TODO add more keywords
	(keyword_token) {
		.lexeme = (string) { .c_str = "return", .len = C_STR_LEN("return"), .cap = C_STR_LEN("return") + 1 },
		.token_kind = KEYWORD_RETURN
	},
	(keyword_token) {
		.lexeme = (string) { .c_str = "int", .len = C_STR_LEN("int"), .cap = C_STR_LEN("int") + 1 },
		.token_kind = KEYWORD_INT
	}
};

token_ptr lexer_tokenize_source(string source_text) {
	// TODO Perhaps log here that lexical analysis starts
	if(source_text.len < 0) {
		return (token_ptr){ .ptr = NULL, .len = -1, .cap = -1 };
	} 
	token_ptr token_array;
	token_array.ptr = malloc(5 * sizeof(*token_array.ptr)); 
	if(token_array.ptr == NULL) {
		log_error("Failed to allocate memory for token array");
		return (token_ptr){ .ptr = NULL, .len = -1, .cap = -1 };
	}
	token_array.len = 0;
	token_array.cap = 5;
	int64_t index = 0;
	while(index < source_text.len) {
		index = next_non_whitespace(source_text, index + 1);
		if(index < 0) {
			// TODO: If we switch to uint64_t then we can replace the
			// call to next_non_whitespace with sv_trim_left. Ending
			// up inside this if-statement when using
			// next_non_whitespace() corresponds to the string_view sv
			// having length zero after calling sv_trim_left(&sv).
			break;
		}
		token (*tokenize_fun_ptr[]) (string, int64_t) = {
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
			token_array = realloc_token_ptr(token_array);
			if(token_array.len == -1) {
				// Note: The old memory was freed in realloc_token_ptr
				log_error("Failed to allocate memory for token array during lexing");
				return (token_ptr){ .ptr = NULL, .len = -1, .cap = -1 };
			}
		}
		// TODO Perhaps make a log entry here: report what remains of
		// the line from index onwards.
		for(int64_t i = 0; i < ARR_LEN(tokenize_fun_ptr); i++) {
			token lex = tokenize_fun_ptr[i](source_text, index);
			if(lex.token_kind != LEXEME_ERROR) {
				identified_token = true;
				token_array.ptr[token_array.len] = lex;
				token_array.len++;
				/* string test = set_string(source_text.c_str + lex.first_index); */
				/* test.c_str[lex.last_index - lex.first_index + 1] = '\0'; */
				/* printf("lexeme: %-10s\tfirst index %ld last index %ld\n", test.c_str, lex.first_index, lex.last_index); */
				/* free_string(test); */
				index = lex.last_index;
				break;
			}
		}
		// TODO Perhaps make a log entry here: if a token was found,
		// report which substring it was and which token, and if a
		// token was not found then report that
		if(!identified_token) {
			token_array.ptr[token_array.len] = (token) {
				.lexeme = (string_view) { .ptr = NULL, .len = 0 },
				.token_kind = LEXEME_ERROR,
				.first_index = index,
				.last_index = index
			};
			token_array.len++;
			/* printf("erroneous: %-7c\tat string index %ld\n", source_text.c_str[index], index); */
		}
	}
	return token_array;
}
 
token_ptr realloc_token_ptr(token_ptr old_ptr) {
	token_ptr new_ptr;
	new_ptr.ptr = malloc(2 * old_ptr.cap * sizeof(*new_ptr.ptr));
	if(new_ptr.ptr == NULL) {
		log_error("Failed to allocate memory for token_ptr");
		free(old_ptr.ptr);
		return (token_ptr){ .ptr = NULL, .len = -1, .cap = -1 };
	}
	new_ptr.cap = 2 * old_ptr.cap;
	new_ptr.len = old_ptr.len;
	for(int64_t j = 0; j < old_ptr.len; j++) {
		new_ptr.ptr[j] = old_ptr.ptr[j];
	}
	free(old_ptr.ptr);
	return new_ptr;
}

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

token tokenize_single_char(string source_text, int64_t index, char c, int32_t token_kind) {
	if(!(source_text.c_str[index] == c)) {
		return (token) {
			.lexeme = (string_view) { .ptr = NULL, .len = 0 },
			.token_kind = LEXEME_ERROR,
			.first_index = -1,
			.last_index = -1
		};
	}
	return (token) {
		.lexeme = (string_view) { .ptr = source_text.c_str + index, .len = 1 },
		.token_kind = token_kind,
		.first_index = index,
		.last_index = index
	};
}

token tokenize_paren_open(string source_text, int64_t index) {
	return tokenize_single_char(source_text, index, '(', PAREN_OPEN);
}

token tokenize_paren_close(string source_text, int64_t index) {
	return tokenize_single_char(source_text, index, ')', PAREN_CLOSE);
}

token tokenize_curly_open(string source_text, int64_t index) {
	return tokenize_single_char(source_text, index, '{', CURLY_OPEN);
}

token tokenize_curly_close(string source_text, int64_t index) {
	return tokenize_single_char(source_text, index, '}', CURLY_CLOSE);
}

token tokenize_semicolon(string source_text, int64_t index) {
	return tokenize_single_char(source_text, index, ';', SEMICOLON);
}

token tokenize_number(string source_text, int64_t index) {
	// TODO add other number representations, such as scientific
	// notation, floating point numbers and numbers with type suffixes
	// like .5f
	if(!isdigit(source_text.c_str[index])) {
		return (token) {
			.lexeme = (string_view) { .ptr = NULL, .len = 0 },
			.token_kind = LEXEME_ERROR,
			.first_index = -1,
			.last_index = -1
		};
	}
	int64_t len = 0;
	while(index + len < source_text.len) {
		if(!isdigit(source_text.c_str[index + len])) {
			break;
		} else {
			len++;
		}
	}
	return (token) {
		.lexeme = (string_view) { .ptr = source_text.c_str + index, .len = len },
		.token_kind = NUMBER,
		.first_index = index,
		.last_index = index + len - 1
	};
}

token tokenize_keyword(string source_text, int64_t index) {
	token result = (token) {
		.lexeme = (string_view) { .ptr = NULL, .len = 0 },
		.token_kind = LEXEME_ERROR,
		.first_index = -1,
		.last_index = -1
	};
	string tmp = set_string(source_text.c_str + index);
	for(int64_t i = 0; i < ARR_LEN(keyword_token_list); i++) {
		if(index + keyword_token_list[i].lexeme.len < source_text.len) {
			char tmp_char = tmp.c_str[keyword_token_list[i].lexeme.len];
			int64_t tmp_len = tmp.len;
			tmp.c_str[keyword_token_list[i].lexeme.len] = '\0';
			tmp.len = keyword_token_list[i].lexeme.len;
			if(str_cmp(keyword_token_list[i].lexeme, tmp)) {
				result = (token) {
					.lexeme = (string_view) { .ptr = source_text.c_str + index, .len = keyword_token_list[i].lexeme.len },
					.token_kind = keyword_token_list[i].token_kind,
					.first_index = index,
					.last_index = index + keyword_token_list[i].lexeme.len - 1
				};
				break;
			}
			tmp.c_str[keyword_token_list[i].lexeme.len] = tmp_char;
			tmp.len = tmp_len;
		}
	}
	free_string(tmp);
	return result;
}

token tokenize_id(string source_text, int64_t index) {
	if(!(isalpha(source_text.c_str[index]) || source_text.c_str[index] == '_')) {
		return (token) {
			.lexeme = (string_view) { .ptr = NULL, .len = 0 },
			.token_kind = LEXEME_ERROR,
			.first_index = -1,
			.last_index = -1
		};
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
	return (token) {
		.lexeme = (string_view) { .ptr = source_text.c_str + index, .len = len },
		.token_kind = IDENTIFIER,
		.first_index = index,
		.last_index = index + len - 1
	};
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

