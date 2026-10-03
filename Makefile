
default: out/main

out/main: out/main.o out/string.o out/file_io.o out/lexer.o out/pointer.o
	gcc -std=c2x -fsanitize=address -g -o $@ $^

out/%.o: src/%.c
	gcc -std=c2x -fsanitize=address -g -o $@ -c $<

PHONY: clean run

clean:
	rm -f ./out/*

run: out/main
	./out/main
