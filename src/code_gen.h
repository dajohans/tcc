#ifndef CODE_GEN_H
#define CODE_GEN_H

#include "ast.h"
#include "parser.h"

typedef struct arithmetic_tree {
	ast* node;
	struct arithmetic_tree* l_child;
	struct arithmetic_tree* r_child;
} arithmetic_tree;

arithmetic_tree* ast_to_arithmetic_tree_product_subtree(ast* expr, ast* out_last_expr);
arithmetic_tree* ast_to_arithmetic_tree(ast* expr);
bool free_arithmetic_tree(arithmetic_tree* at);
string add_code_function_label(ast* identifier);
string add_code_expression(ast* expr);
string add_code_return_statement(ast* statement);
string add_code_function(ast* fun);
string ast_to_assembly(ast* syntax_tree);
void code_gen(ast* syntax_tree);

#endif // CODE_GEN_H
