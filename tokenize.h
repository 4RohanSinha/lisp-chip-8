#ifndef TOKENIZE_H
#define TOKENIZE_H

enum {
	T_OPEN_PAREN, T_CLOSE_PAREN
};

struct token {
	int tokentype;
	int val;
};

int scan(struct token*);

#endif
