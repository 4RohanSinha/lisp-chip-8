HEADERS := c8.h ast.h stmt.h sym.h tokenize.h misc.h mem.h stmt_eval.h sym.h
C_FILE := main.c misc.c stmt.c tokenize.c stmt_eval.c c8.c sym.c
OBJ := main.o misc.o stmt.o tokenize.o stmt_eval.o c8.o sym.o
CC := gcc
ASAN_FLAGS := -fsanitize=address

lispc-c8: $(OBJ)
	$(CC) -o $@ $(ASAN_FLAGS) $^

main.o: main.c $(HEADERS)
sym.o: sym.c $(HEADERS)
stmt_eval.o: stmt_eval.c $(HEADERS)
c8.o: c8.c $(HEADERS)
misc.o: misc.c misc.h
stmt.o: stmt.c $(HEADERS)
tokenize.o: tokenize.c $(HEADERS)

clean:
	rm -f lispc-c8
	rm -f $(OBJ)
	rm -f assembler
	rm -f assembler.o

assembler: assembler.o
	$(CC) -o $@ $(ASAN_FLAGS) $^

assembler.o: assembler.c assembler.h
