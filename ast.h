#ifndef AST_H
#define AST_H

#define MAX_CHILDREN 10
enum {
	A_INTLIT, A_ADD, A_SUB, A_MULT, A_DIV
};

struct ast_node {
	int t_operatortype;
	struct ast_node* children[MAX_CHILDREN];
	int kChildren;

	union {
		int val;
		char* symbol;
	} key;
};

#endif
