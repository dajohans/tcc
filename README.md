
# Torsö C Compiler (TCC)

In case anybody encounters this repo: This is just my attempt at getting back into programming.
I have long wanted to implement a compiler, so I'm implementing a compiler.
The project name is based on a Swedish island close to my heart: [Torsö](https://en.wikipedia.org/wiki/Torsö).

## Goals

The goals are:
- compile C to assembly,
- self-hosting: the compiler can compile itself,
- implement everything from scratch: no use of compiler tools like Yacc or LLVM.

The basic steps that need to be implemented:
- preprocessor
- lexical analysis
- parser
- semantic analysis
- code generation

Optional extras if I feel ambitious:
- intermediate representation, instead of compiling straight to assembly
- optimizing the generated code
- error messages

## Non-goals

Linking as well as compilation of assembly to binary are out of scope.

## Current goal

- [ ] Adapt the code generation to properly use the AST

## Completed goals

- [x] Adapt type checking in semantic analysis to the new type table
- [x] Improve the parser and lexer designs
    - [x] Specify grammar rules as static data, rather than each grammar rule being implemented as a separate C function
    - [x] Add functions for traversing, accessing data from, and modifying the token list and AST
    - [x] Rethink the type situation. New approach: Each AST node for a scope now contains a type table. Parsing is done in two phases. First phase is a pure context free grammar and ignores type tables. The second phase updates type tables. Refactored AST data structure to fit this model better.    
    - [x] Think about the keywork situation. Conclusion: Each keyword should probably keep having its own token. Having one keyword token for all builtin types could make some parser implementations simpler, but with the current type table approach it makes no difference. Separating the keyword 'if' from keyword 'for' makes sense in the parser.
- [x] Add initial unit testing on string.c
- [x] Adapt the minimal viable product implementation to not be hard coded for the example C program.
    - [x] Enable substring handling by something like C++'s string_view
    - [x] Make the lexical tokens and abstract syntax tree nodes contain their corresponding lexemes as a string_view
    - [x] Add semantic analysis step: check the function return type in the AST matches the type of the actual return value
    - [x] Adapt the code generation to obtain the function name main and the return value 0 from the lexemes stored in the AST
- [x] Implement the most minimal viable product possible.
The compiler can compile the following simple C program to assembly
```C
int main() {
	return 0;
}
```
This implementation goes through lexing, grammar parsing and code generation, but in a truly minimal manner.
For example, the code generator does not obtain the function name main by looking at the abstract syntax tree.
But the basic infrastructure for each compilation step is there, except semantic analysis.

## Dependencies

- [GCC](https://gcc.gnu.org/)
- [Make](https://www.gnu.org/software/make/)


