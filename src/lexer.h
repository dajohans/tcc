#ifndef LEXER_H
#define LEXER_H

#include <stdint.h>

#include "pointer.h"
#include "string_view.h"

enum Token_Kinds {
	LEXEME_ERROR = -1,
	IDENTIFIER,
	KEYWORD_INT,
	KEYWORD_FLOAT,
	KEYWORD_RETURN,
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

typedef struct token {
	string_view lexeme;
	int32_t token_kind;
	int64_t first_char_index;
	int64_t last_char_index;
	// If char* str contains the source code file, then
	// str[first_char_index] is the first character of the lexeme and
	// str[last_char_index] is the last character.
} token;

typedef struct token_ptr {
	token* ptr;
	int64_t len;
	int64_t cap;
} token_ptr;

token_ptr lexer_tokenize_source(string_view sv);
token_ptr realloc_token_ptr(token_ptr old_ptr);
char* token_to_c_str(int32_t token);
token tokenize_single_char(int64_t index, string_view sv, char c, int32_t token_kind);
token tokenize_paren_open(int64_t index, string_view sv);
token tokenize_paren_close(int64_t index, string_view sv);
token tokenize_curly_open(int64_t index, string_view sv);
token tokenize_curly_close(int64_t index, string_view sv);
token tokenize_semicolon(int64_t index, string_view sv);
token tokenize_number(int64_t index, string_view sv);
token tokenize_keyword(int64_t index, string_view sv);
token tokenize_id(int64_t index, string_view sv);
bool token_is_id_or_type(token tok);
token uninit_lex_token();

#endif // LEXER_H
