
# Torsö C Compiler (TCC)

This is just my attempt at getting back into programming, by implementing a project I have long wanted to do.
[Torsö](https://en.wikipedia.org/wiki/Torsö) refers t

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

Implement a basic version of lexer, parser, semantic analyser and code generator so that the following simple C program can be compiled
```C
int main() {
	return 0;
}
```

## Dependencies

- [GCC](https://gcc.gnu.org/)
- [Make](https://www.gnu.org/software/make/)


