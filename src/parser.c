
/* The goal is to parse the following
 *     int main() {
 *         return 0;
 *     }
 * To to this, I suppose we need the rules:
 *     statement -> keyword_return + number + semicolon
 *     function_prototype -> identifier + identifier + paren_open + paren_close
 *     function_definition -> function_prototype + curly_open + statement + curle_close
 * I think the idea will be to build an abstract syntax tree as
 * follows. Each left-hand side in the grammer rule is a node, and it
 * will have one child for each term in the right-hand side. If the
 * term in the right-hand side is a lexeme such as keyword_return then
 * that child is a leaf node, and if the term is itself a left-hand
 * side of some rule then it will not be a leaf node.
 *
 * All the statements in a given scope will be children of the
 * language construct which owns the scope (if-statement, for-loop,
 * function definition etc). The top level node does not correspond to
 * any language construct in the file and may be thought of as
 * corresponding to the file itself. The children of the top-level
 * node are things like include directives, function declarations and
 * definitions, global variable definitions.
 */

 
 
 
