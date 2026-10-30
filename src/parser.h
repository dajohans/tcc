#ifndef PARSER_H
#define PARSER_H

#include "ast.h"
#include "lexer.h"
#include "pointer.h" // Probably remove this later

bool is_grammar_rule(ast* (rule) (token_ptr, int64_t));
bool is_scope_rule(syntax_kind rule);
syntax_kind get_rule_kind(ast* (rule) (token_ptr, int64_t));
bool parse_current_rule(token_ptr tokens, int64_t* index, int64_t rule_index, ast* rule_top_node);
ast* parse_rule_kind(token_ptr tokens, int64_t* index, ast* (rule) (token_ptr, int64_t));
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
ast* parse_type_identifier(token_ptr tokens, int64_t index);
bool is_type_in_table(type_table types, ast* type_node);
bool type_exists(ast* type_node);
type_info find_type_in_table(type_table types, ast* type_node);
type_info find_type(ast* type_node);
bool parser_validate_types(ast* syntax_tree);
bool determine_expression_type(ast* expr);
bool parser_assign_expression_types(ast* syntax_tree);

#endif // PARSER_H

