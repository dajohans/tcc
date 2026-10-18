#include <stdio.h>

#include "lexer.h"
#include "parser.h"
#include "string_view.h"

typedef struct type_info {
	int64_t id;
	string_view name;
} type_info;

bool return_type_check(ast* syntax_tree) {
	if(syntax_tree->kind != NON_TERMINAL) {
		return false;
	}
	if(syntax_tree->node.syntax.construct != FUNCTION_DEFINITION) {
		return false;
	}
	// TODO: This table should be initialized elsewhere. Perhaps make
	// sure the table has a value to signal that there's an error,
	// probably with .id = -1
	string int_str = make_string("int");
	if(int_str.cap < 0) {
		return false;
	}
	string float_str = make_string("float");
	if(float_str.cap < 0) {
		free_string(&int_str);
		return false;
	}
	string_view int_sv = set_sv_ptr(int_str.c_str, int_str.len);
	string_view float_sv = set_sv_ptr(float_str.c_str, float_str.len);
	type_info type_table[] = {
		(type_info) { .id = KEYWORD_INT, .name = int_sv },
		(type_info) { .id = KEYWORD_FLOAT, .name = float_sv }
	};
	type_info function_return_type;
	for(int64_t i = 0; i < ARR_LEN(type_table); i++) {
		if(syntax_tree->node.syntax.children[0]->node.syntax.children[0]->node.syntax.children[0]->node.lex_token.token_kind == type_table[i].id) {
			function_return_type = type_table[i];
		}
	}
	type_info return_value_type;
	for(int64_t i = 0; i < syntax_tree->node.syntax.children_count; i++){
		if(syntax_tree->node.syntax.children[i]->kind == TERMINAL) {
			continue;
		}
		if(syntax_tree->node.syntax.children[i]->node.syntax.construct != RETURN_STATEMENT) {
			continue;
		}
		for(int64_t l = 0; l < syntax_tree->node.syntax.children[i]->node.syntax.children_count; l++) {
			if(syntax_tree->node.syntax.children[i]->node.syntax.children[l]->node.lex_token.token_kind == KEYWORD_RETURN) {
				if(syntax_tree->node.syntax.children[i]->node.syntax.children[l + 1]->node.lex_token.token_kind == NUMBER) {
					// TODO: This should probably be looked up in a
					// symbol table with type info or something
					return_value_type = type_table[0];
					// TODO: Keep an array of types for all return
					// statements found. Of course there can be more
					// than one...
					break;
				}
			}
		}
		
	}
	free_string(&int_str);
	free_string(&float_str);
	if(function_return_type.id != return_value_type.id) {
		return false;
	}
	return true;
}
