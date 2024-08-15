#include "tokenize.h"
#include "misc.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define IDENT_LEN 1024

static FILE* in_file;
static int numParen = 0;
static int lineNo = 0;
static char rewind_ = 0;
static char ident[IDENT_LEN];

char* get_identifier() { return ident; }
int get_lineNo() { return lineNo; }
int getNumParen() { return numParen; }

void setFile(FILE* f) { in_file = f; }

static int nextcharin(char* s, int c) {
	char* p;
	p = strchr(s, c);
	return (p ? p - s : -1);
}

static int next() {
	int c;

	if (rewind_) {
		c = rewind_;
		rewind_ = 0;
		return c;
	}

	c = fgetc(in_file);
	if ('\n' == c) lineNo++;
	return c;
}

static void putback(int c) {
	rewind_ = c;
}


static int skip() {
	int c;
	c = next();

	//while ((' ' == c || '\t' == c || '\n' == c || '\r' == c || '\f' == c)) {
	while (isspace(c)) {
		c = next();
	}

	return c;
}

static int scanint(int c) {
	int k, val = 0;
	
	while ((k = nextcharin("0123456789", c)) >= 0) {
		val = val * 10 + k;
		c = next();
	}

	if (!isspace(c) && c != EOF && c != ')') {
		printf("Illegal token following number: %d on line %d\n", val, lineNo);
		exit(1);
	}

	putback(c);
	return val;
}

static int scan_identifier(int c, char* buffer, int max_len) {
	int len = 0;

	//while (isalpha(c) || isdigit(c) || c == '_') {
	while (!isspace(c) && c != EOF && c != ')') {
		if (max_len - 1 == len) {
			printf("Identifier too long\n");
		} else if (len < max_len - 1) {
			buffer[len++] = c;
		}
		
		c = next();
	}

	putback(c);
	buffer[len] = '\0';
	return len;
}

static int keyword(char* s) {
	switch (s[0]) {
		case '+':
			if (!strcmp(s, "+")) return T_PLUS;
		case '-':
			if (!strcmp(s, "-")) return T_MINUS;
		case '*':
			if (!strcmp(s, "*")) return T_STAR;
		case '/':
			if (!strcmp(s, "/")) return T_SLASH;
		case 'l':
			if (!strcmp(s, "let")) return T_LET;
		case 's':
			if (!strcmp(s, "setq")) return T_SETQ;
		case 'c':
			if (!strcmp(s, "cls")) return T_CLS;
		case '=':
			if (!strcmp(s, "=")) return T_EQUALS;
		case '>':
			if (!strcmp(s, ">")) return T_GT;
			if (!strcmp(s, ">=")) return T_GTE;
		case '<':
			if (!strcmp(s, "<")) return T_LT;
			if (!strcmp(s, "<=")) return T_LTE;
		default:
			return T_UNKNOWN;
	}
}

int scan(struct token* t) {
	int c = skip();
	switch (c) {
		case '(':
			numParen++;
			t->tokentype = T_OPEN_PAREN;
			break;
		case ')':
			if (numParen == 0) printf("Unexpected closing parenthesis on line %d\n", lineNo);
			numParen--;
			t->tokentype = T_CLOSE_PAREN;
			break;
		case EOF:
			if (numParen > 0) printf("Unclosed parenthesis\n");
			t->tokentype = T_EOF;
			return 0;
		default:
			if (isdigit(c)) {
				int val = scanint(c);
				t->tokentype = T_INTLIT;
				t->val.intval = val;
				break;
			} else {
				scan_identifier(c, ident, IDENT_LEN);
				int ttype;
				if ((ttype = keyword(ident))) { 
					t->tokentype = ttype;
					break;
				}

				t->tokentype = T_IDENT;
				struct symtableloc sloc = sym_process_symbol(ident);
				t->val.sloc = sloc;
			}


	}

	return 1;
}

void match(struct token* t, int type, char* rep) {
	if (t->tokentype != type) {
		printf("Error: Expected %s on line %d\n", rep, lineNo);
		exit(1);
	}

	scan(t);
}

void match_multiple(struct token* t, int types[], int nTypes, char* rep) {
	for (int i = 0; i < nTypes; i++) {
		if (t->tokentype == types[i]) {
			scan(t);
			return;
		}
	}

	printf("Unexpected token on line %d\n", lineNo);
	exit(1);
}
