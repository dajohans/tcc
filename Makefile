CC = gcc
CFLAGS = -std=c2x -Wall -Wextra -g -fsanitize=address,undefined,leak,pointer-compare,pointer-subtract

source := out/main.o out/string_view.o out/file_io.o out/lexer.o out/pointer.o out/error.o out/parser.o out/code_gen.o out/semantic.o out/ast.o
testing_source := $(filter-out out/main.o, $(source))
testing_source += tests/test_main.c


default: out/main

out/main: $(source)
	$(CC) $(CFLAGS) -o $@ $^

out/%.o: src/%.c
	$(CC) $(CFLAGS) -o $@ -c $<

PHONY: clean run tests

tests: tests/test_main
# We test for malloc returning null, so need to tell the sanitizer to
# not terminate the program when that happens
	ASAN_OPTIONS=allocator_may_return_null=1 ./$<
	rm -f $<

tests/test_main: $(testing_source)
	gcc $(CFLAGS) -I./src -L./out $^ -o $@

assembly:
	gcc -c a.s -o a.o
	ld -o a.out a.o
	./a.out

clean:
	rm -f ./out/* a.s a.o a.out

run: out/main
	./out/main
