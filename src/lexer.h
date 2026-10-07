#ifndef LEXER_H
#define LEXER_H

#include <stdint.h>

#include "pointer.h"
#include "string.h"

enum Lexer_Tokens {
	LEXEME_ERROR = -1,
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

typedef struct lexeme {
	int32_t token;
	int64_t first_index;
	int64_t last_index;
} lexeme;

char* token_to_c_str(int32_t token);
lexeme tokenize_single_char(string source_text, int64_t index, char c, int32_t token);
lexeme tokenize_paren_open(string source_text, int64_t index);
lexeme tokenize_paren_close(string source_text, int64_t index);
lexeme tokenize_curly_open(string source_text, int64_t index);
lexeme tokenize_curly_close(string source_text, int64_t index);
lexeme tokenize_semicolon(string source_text, int64_t index);
lexeme tokenize_number(string source_text, int64_t index);
lexeme tokenize_keyword(string source_text, int64_t index);
lexeme tokenize_id(string source_text, int64_t index);
int64_t next_non_whitespace(string source_text, int64_t start_index);
int32_ptr lexer_tokenize_source(string source_text);

#endif // LEXER_H
