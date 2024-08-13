#include "sym.h"

#include <stdlib.h>
#include <string.h>
#include "c8.h"

static int nGlobals = 0;
static int nLocals = 0;

struct symbol symtable[NSYMBOLS];

struct symbol resolve_gsymbol(char* symbol) {
	struct symbol s;

	for (int i = 0; i < nGlobals; i++) {
		int ind = NSYMBOLS-1-i;
		if (!strcmp(symbol, symtable[ind].name))
			return symtable[ind];
	}

	s.i_type = I_NONE;
	s.loc.type = L_NONE;
	return s;
}

struct location add_gsymbol(char* symbol) {
	struct symbol new_sym;

	struct location loc;
	int reg = c8_alloc_reg();
	loc.type = L_REG;
	loc.loc = reg;

	new_sym.loc = loc;
	new_sym.i_type = I_VAR;
	new_sym.name = (char*)(malloc(sizeof(char)*(strlen(symbol)+1)));
	strcpy(new_sym.name, symbol);
	symtable[NSYMBOLS-1-nGlobals] = new_sym;
	nGlobals++;

	return loc;
}
