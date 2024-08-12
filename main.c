#include "tokenize.h"
#include "stmt.h"
#include "c8.h"
#include <string.h>
#include <stdio.h>
#include <errno.h>

/*
const char* tokenToString(int token) {
    switch (token) {
        case T_OPEN_PAREN: return "T_OPEN_PAREN";
        case T_CLOSE_PAREN: return "T_CLOSE_PAREN";
        case T_PLUS: return "T_PLUS";
        case T_MINUS: return "T_MINUS";
        case T_STAR: return "T_STAR";
        case T_SLASH: return "T_SLASH";
        case T_INTLIT: return "T_INTLIT";
        case T_SETQ: return "T_SETQ";
        case T_LET: return "T_LET";
        case T_IDENT: return "T_IDENT";
        case T_EOF: return "T_EOF";
        default: return "UNKNOWN_TOKEN";
    }
}*/

int main(int argc, char* argv[]) {
	FILE* f = fopen(argv[1], "r");
	if (f == NULL) {
		fprintf(stderr, "Unable to open %s: %s\n", argv[1], strerror(errno));
		return 1;
	}
	c8_init("out.s");
	setFile(f);
	st_parse();
/*
	struct token t;
	while (scan(&t)) {
		printf("%s\n", tokenToString(t.tokentype));
		printf("\n");
	}*/
	fclose(f);
}
