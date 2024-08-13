#ifndef TOKENIZE_H
#define TOKENIZE_H

#include <stdio.h>

enum {
	T_UNKNOWN, T_OPEN_PAREN, T_CLOSE_PAREN, 
	T_PLUS, T_MINUS, T_STAR, T_SLASH, T_INTLIT,
	T_SETQ, T_LET, T_IDENT, T_EOF, T_RVALUE
};

struct token {
	int tokentype;

	union {
		int intval;
		char* charval;
	} val;

};

void setFile(FILE*);
char* get_identifier();
int get_lineNo();
int getNumParen();
int scan(struct token*);
void match(struct token*, int type, char* rep);
void match_multiple(struct token*, int types[], int nTypes, char* rep);

#endif
