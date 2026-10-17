
# Torsö C Compiler (TCC)

This is just my attempt at getting back into programming, by implementing a project I have long wanted to do.
[Torsö](https://en.wikipedia.org/wiki/Torsö) is an island in Sweden.

## Goals

The goals are:
- compile C to assembly,
- self-hosting: the compiler can compile itself,
- implement everything from scratch: no use compiler tools like Yacc or LLVM.

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

- [ ] Improve the parser and lexer designs
    - [ ] Specify grammar rules as static data, rather than each grammar rule being implemented as a separate C function
    - [ ] Add functions for traversing, accessing data from, and modifying the token list and AST
    - [ ] Rethink the type situation. Types can be defined at arbitrary points in the source code, so the type cannot strictly speaking be determined by a static context free grammar. Types need to be checked against a dynamic type table.    
    - [ ] Think about the keywork situation. Should each keyword be its own token or should there only be one keyword token? 

## Completed goals

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


