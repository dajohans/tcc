#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "ast.h"
#include "parser.h"

bool analyze_semantics(ast* syntax_tree);
bool return_type_check(ast* syntax_tree);

#endif // SEMANTIC_H
