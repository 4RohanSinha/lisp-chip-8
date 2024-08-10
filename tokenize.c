#include "tokenize.h"
#include "misc.h"

static int numParen = 0;
static int lineNo = 0;


int scan(struct token* t) {
	int c = getNextChar();

	switch (c) {
		case '(':
			numParen++;
			t->tokentype = T_OPEN_PAREN;
			break;
		case ')':
			if (numParen == 0) syntaxerror("Unexpected closing parenthesis on line %d\n", lineNo);
			numParen--;
			t->tokentype = T_CLOSE_PAREN;
			break;
		case EOF:
			if (numParen > 0) syntaxerror("Unclosed parenthesis%d\n");
			t->tokentype = T_EOF;
			break;
		default:

	}
}
