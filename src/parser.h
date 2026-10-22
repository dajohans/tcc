#ifndef PARSER_H
#define PARSER_H

#include "ast.h"
#include "lexer.h"
#include "pointer.h" // Probably remove this later

bool is_grammar_rule(ast* (rule) (token_ptr, int64_t));
bool is_scope_rule(syntax_kind rule);
syntax_kind get_rule_kind(ast* (rule) (token_ptr, int64_t));
ast* parse_rule(token_ptr tokens, int64_t* index, ast* (rule) (token_ptr, int64_t));
ast* parse_top_level(token_ptr tokens, int64_t index);
ast* parse_identifier(token_ptr tokens, int64_t index);
ast* parse_close_curly(token_ptr tokens, int64_t index);
ast* parse_open_curly(token_ptr tokens, int64_t index);
ast* parse_close_paren(token_ptr tokens, int64_t index);
ast* parse_open_paren(token_ptr tokens, int64_t index);
ast* parse_keyword_type(token_ptr tokens, int64_t index);
ast* parse_keyword_return(token_ptr tokens, int64_t index);
ast* parse_number(token_ptr tokens, int64_t index);
ast* parse_semicolon(token_ptr tokens, int64_t index);
ast* parse_type(token_ptr tokens, int64_t index);

#endif // PARSER_H
