#ifndef CODE_GEN_H
#define CODE_GEN_H

#include "ast.h"
#include "parser.h"

string add_code_function_label(ast* identifier);
string add_code_expression(ast* expr);
string add_code_return_statement(ast* statement);
string add_code_function(ast* fun);
string ast_to_assembly(ast* syntax_tree);
void code_gen(ast* syntax_tree);

#endif // CODE_GEN_H
