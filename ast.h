#ifndef AST_H
#define AST_H

enum {
	A_ADD, A_SUB, A_MULT, A_DIV
};

struct ast_node {
	int tokentype;
	int val;
};

#endif
