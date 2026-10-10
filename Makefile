CC = gcc
CFLAGS = -std=c2x

default: out/main

out/main: out/main.o out/string.o out/file_io.o out/lexer.o out/pointer.o out/error.o out/parser.o out/code_gen.o
	$(CC) $(CFLAGS) -fsanitize=address -g -o $@ $^

out/%.o: src/%.c
	$(CC) $(CFLAGS) -fsanitize=address -g -o $@ -c $<

PHONY: clean run

clean:
	rm -f ./out/*

run: out/main
	./out/main
