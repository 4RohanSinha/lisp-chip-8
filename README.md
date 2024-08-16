# Lisp: Chip 8 Flavor


This project seeks to implement a compiler that will create Assembly instructions for the old Chip 8 gaming console from a Lisp program. Still under development! Currently constructs and prints AST.

# Usage

To run this project's compiler into Assembly:
```
make
./lispc-c8 [FILENAME]
```

To run this project's assembler converting Chip 8 Assembly into a ROM file:
```
make assembler
./assembler out.s
```

This outputs a Chip 8 ROM file.
