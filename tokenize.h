#ifndef TOKENIZE_H
#define TOKENIZE_H

#include <stdio.h>
#include "sym.h"

enum {
	T_UNKNOWN,

	T_EOF,
	
	T_OPEN_PAREN, T_CLOSE_PAREN, 

	T_PLUS, T_MINUS, T_STAR, T_SLASH, T_INTLIT,

	T_SETQ, T_LET, T_IDENT, T_CLS,

	T_EQUALS, T_LT, T_GT, T_LTE, T_GTE,

	T_IF
};

struct token {
	int tokentype;

	union {
		int intval;
		struct symtableloc sloc;		
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
