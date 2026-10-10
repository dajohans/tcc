CC = gcc
CFLAGS = -std=c2x -Wall -Wextra -g -fsanitize=address,undefined,leak,pointer-compare,pointer-subtract

default: out/main

out/main: out/main.o out/string.o out/file_io.o out/lexer.o out/pointer.o out/error.o out/parser.o out/code_gen.o
	$(CC) $(CFLAGS) -o $@ $^

out/%.o: src/%.c
	$(CC) $(CFLAGS) -o $@ -c $<

PHONY: clean run

assembly:
	gcc -c a.s -o a.o
	ld -o a.out a.o
	./a.out

clean:
	rm -f ./out/* a.s a.o a.out

run: out/main
	./out/main
