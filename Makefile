HEADERS := c8.h ast.h stmt.h sym.h tokenize.h misc.h mem.h stmt_eval.h
C_FILE := main.c misc.c stmt.c tokenize.c stmt_eval.c c8.c
OBJ := main.o misc.o stmt.o tokenize.o stmt_eval.o c8.o
CC := gcc

interp: $(OBJ)
	$(CC) -o $@ $^

main.o: main.c $(HEADERS)
stmt_eval.o: stmt_eval.c $(HEADERS)
c8.o: c8.c $(HEADERS)
misc.o: misc.c misc.h
stmt.o: stmt.c $(HEADERS)
tokenize.o: tokenize.c $(HEADERS)

clean:
	rm -f interp
