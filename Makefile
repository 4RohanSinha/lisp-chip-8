HEADERS := c8.h ast.h stmt.h sym.h tokenize.h misc.h mem.h 
C_FILE := main.c misc.c stmt.c tokenize.c
OBJ := main.o misc.o stmt.o tokenize.o
CC := gcc

interp: $(OBJ)
	$(CC) -o $@ $^

main.o: main.c $(HEADERS)
misc.o: misc.c misc.h
stmt.o: stmt.c $(HEADERS)
tokenize.o: tokenize.c $(HEADERS)

clean:
	rm -f interp
