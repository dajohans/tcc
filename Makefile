CC = gcc
CFLAGS = -std=c2x -Wall -Wextra -g -fsanitize=address,undefined,leak,pointer-compare,pointer-subtract

default: out/main

out/main: out/main.o out/string_view.o out/file_io.o out/lexer.o out/pointer.o out/error.o out/parser.o out/code_gen.o out/semantic.o
	$(CC) $(CFLAGS) -o $@ $^

out/%.o: src/%.c
	$(CC) $(CFLAGS) -o $@ -c $<

PHONY: clean run tests

tests: tests/test_main
	./$<
	rm -f $<

tests/test_main: tests/test_main.c out/string_view.o out/file_io.o out/lexer.o out/pointer.o out/error.o out/parser.o out/code_gen.o out/semantic.o
	gcc $(CFLAGS) -I./src -L./out $^ -o $@

assembly:
	gcc -c a.s -o a.o
	ld -o a.out a.o
	./a.out

clean:
	rm -f ./out/* a.s a.o a.out

run: out/main
	./out/main
