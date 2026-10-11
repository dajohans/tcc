#ifndef LEXER_H
#define LEXER_H

#include <stdint.h>

#include "pointer.h"
#include "string.h"

enum Token_Kinds {
	LEXEME_ERROR = -1,
	IDENTIFIER,
	KEYWORD_INT,
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
	int64_t first_index;
	int64_t last_index;
} token; // TODO: Change name to token

typedef struct token_ptr {
	token* ptr;
	int64_t len;
	int64_t cap;
} token_ptr;

token_ptr lexer_tokenize_source(string source_text);
token_ptr realloc_token_ptr(token_ptr old_ptr);
char* token_to_c_str(int32_t token);
token tokenize_single_char(string source_text, int64_t index, char c, int32_t token_kind);
token tokenize_paren_open(string source_text, int64_t index);
token tokenize_paren_close(string source_text, int64_t index);
token tokenize_curly_open(string source_text, int64_t index);
token tokenize_curly_close(string source_text, int64_t index);
token tokenize_semicolon(string source_text, int64_t index);
token tokenize_number(string source_text, int64_t index);
token tokenize_keyword(string source_text, int64_t index);
token tokenize_id(string source_text, int64_t index);
int64_t next_non_whitespace(string source_text, int64_t start_index);

#endif // LEXER_H
