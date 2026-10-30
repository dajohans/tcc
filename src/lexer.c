#include <ctype.h>
#include <stdlib.h>

#include "error.h"
#include "lexer.h"
#include "pointer.h"

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
 *     / *[*]**/
/* where [*] means any symbol, and the space between / and * should be
 * removed. But removing it leads to compiler warnings... * Lastly,
 * header name is something like
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

typedef struct keyword_token {
	string lexeme;
	int32_t token_kind;
} keyword_token;

const keyword_token keyword_token_list[] = {
	// TODO: Somehow consolidate this list and the enum with the
	// corresponding integer codes in lexer.h. I now tried to extend
	// the keyword list, and came across strange bugs caused by me not
	// remembering to update this list accordingly...
	(keyword_token) {
		.lexeme = (string) { .c_str = "int", .len = C_STR_LEN("int"), .cap = C_STR_LEN("int") + 1 },
		.token_kind = KEYWORD_INT
	},
	(keyword_token) {
		.lexeme = (string) { .c_str = "float", .len = C_STR_LEN("float"), .cap = C_STR_LEN("float") + 1 },
		.token_kind = KEYWORD_FLOAT
	},
	(keyword_token) {
		.lexeme = (string) { .c_str = "return", .len = C_STR_LEN("return"), .cap = C_STR_LEN("return") + 1 },
		.token_kind = KEYWORD_RETURN
	}
};

token_ptr lexer_tokenize_source(string_view sv) {
	// TODO Perhaps log here that lexical analysis starts
	if(sv_is_empty(sv)) {
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
	while(!sv_is_empty(sv)) {
		string_view trimmed = sv_trim_left(&sv);
		index += trimmed.len;
		if(sv_is_empty(sv)) {
			// TODO: If we switch to uint64_t then we can replace the
			// call to next_non_whitespace with sv_trim_left. Ending
			// up inside this if-statement when using
			// next_non_whitespace() corresponds to the string_view sv
			// having length zero after calling sv_trim_left(&sv).
			break;
		}
		token (*tokenize_fun_ptr[]) (int64_t, string_view) = {
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
			token lex = tokenize_fun_ptr[i](index, sv);
			if(lex.token_kind != LEXEME_ERROR) {
				identified_token = true;
				token_array.ptr[token_array.len] = lex;
				token_array.len++;
				break;
			}
		}
		// TODO Perhaps make a log entry here: if a token was found,
		// report which substring it was and which token, and if a
		// token was not found then report that
		if(!identified_token) {
			token_array.ptr[token_array.len] = (token) {
				.lexeme = (string_view) { .ptr = sv.ptr, .len = 1 },
				.token_kind = LEXEME_ERROR,
				.first_char_index = index,
				.last_char_index = index
			};
			token_array.len++;
		}
		index += token_array.ptr[token_array.len-1].lexeme.len;
		sv_move_forward(&sv, token_array.ptr[token_array.len-1].lexeme.len);
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
		case KEYWORD_FLOAT:
			str = "KEYWORD_FLOAT";
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

token tokenize_single_char(int64_t index, string_view sv, char c, int32_t token_kind) {
	if(sv_is_empty(sv) || sv.ptr[0] != c) {
		return uninit_lex_token();
	}
	return (token) {
		.lexeme = (string_view) { .ptr = sv.ptr, .len = 1 },
		.token_kind = token_kind,
		.first_char_index = index,
		.last_char_index = index
	};
}

token tokenize_paren_open(int64_t index, string_view sv) {
	return tokenize_single_char(index, sv, '(', PAREN_OPEN);
}

token tokenize_paren_close(int64_t index, string_view sv) {
	return tokenize_single_char(index, sv, ')', PAREN_CLOSE);
}

token tokenize_curly_open(int64_t index, string_view sv) {
	return tokenize_single_char(index, sv, '{', CURLY_OPEN);
}

token tokenize_curly_close(int64_t index, string_view sv) {
	return tokenize_single_char(index, sv, '}', CURLY_CLOSE);
}

token tokenize_semicolon(int64_t index, string_view sv) {
	return tokenize_single_char(index, sv, ';', SEMICOLON);
}

token tokenize_number(int64_t index, string_view sv) {
	// TODO add other number representations, such as scientific
	// notation, floating point numbers and numbers with type suffixes
	// like .5f
	if(sv_is_empty(sv) || !isdigit(sv.ptr[0])) {
		return uninit_lex_token();
	}
	int64_t len = 0;
	while(len < sv.len) {
		if(!isdigit(sv.ptr[len])) {
			break;
		} else {
			len++;
		}
	}
	return (token) {
		.lexeme = (string_view) { .ptr = sv.ptr, .len = len },
		.token_kind = NUMBER,
		.first_char_index = index,
		.last_char_index = index + len - 1
	};
}

token tokenize_keyword(int64_t index, string_view sv) {
	if(sv_is_empty(sv)) {
		return uninit_lex_token();
	}
	token result = uninit_lex_token();
	for(int64_t i = 0; i < ARR_LEN(keyword_token_list); i++) {
		if(keyword_token_list[i].lexeme.len <= sv.len) {
			string_view tmp_src = (string_view) {
				.ptr = sv.ptr,
				.len = keyword_token_list[i].lexeme.len
			};
			string_view tmp_keyword = (string_view) {
				.ptr = keyword_token_list[i].lexeme.c_str,
				.len = keyword_token_list[i].lexeme.len
			};
			if(sv_cmp(tmp_src, tmp_keyword)) {
				result = (token) {
					.lexeme = (string_view) { .ptr = sv.ptr, .len = keyword_token_list[i].lexeme.len },
					.token_kind = keyword_token_list[i].token_kind,
					.first_char_index = index,
					.last_char_index = index + keyword_token_list[i].lexeme.len - 1
				};
				break;
			}
		}
	}
	return result;
}

token tokenize_id(int64_t index, string_view sv) {
	if(sv_is_empty(sv) || !(isalpha(sv.ptr[0]) || sv.ptr[0] == '_')) {
		return uninit_lex_token();
	}
	int64_t len = 0;
	while(len < sv.len) {
		if(!(isalnum(sv.ptr[len]) || sv.ptr[len] == '_')) {
			break;
		} else {
			len++;
		}
	}
	// TODO Check if there's an edge case to deal with when index + len == source_text.len
	// For unit tests, make sure to test when both first and last
	// characters of the file are part of an identifier
	return (token) {
		.lexeme = (string_view) { .ptr = sv.ptr, .len = len },
		.token_kind = IDENTIFIER,
		.first_char_index = index,
		.last_char_index = index + len - 1
	};
}

bool token_is_id_or_type(token tok) {
	if(tok.token_kind != IDENTIFIER &&
	tok.token_kind != KEYWORD_INT &&
	tok.token_kind != KEYWORD_FLOAT) {
		return false;
	}
	return true;
}

token uninit_lex_token() {
	return (token) {
		.lexeme = uninit_sv(),
		.token_kind = LEXEME_ERROR,
		.first_char_index = -1,
		.last_char_index = -1
	};
}
