#include "sym.h"
#include "tokenize.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "c8.h"

static int nGlobals = 0;
static int nLocals = 0;
static int nUnresolved = 0;

struct symbol symtable[NSYMBOLS];
struct unresolved_symbol unresolved_symbols[NSYMBOLS];

struct symbol search_symbol(char* symbol) {
	struct symbol s;

	for (int i = 0; i < nGlobals; i++) {
		int ind = NSYMBOLS-1-i;
		if (!strcmp(symbol, symtable[ind].name))
			return symtable[ind];
	}

	s.i_type = I_NONE;
	s.loc.type = L_NONE;
	s.index = -1;
	return s;
}

struct symbol resolve_symbol(struct symtableloc sloc) {
	char* sym = sym_get_symbol_for(sloc);
	return search_symbol(sym);
}

struct location add_symbol(char* symbol) {
	struct symbol new_sym;

	struct location loc;
	int reg = c8_alloc_reg();
	loc.type = L_REG;
	loc.loc = reg;

	new_sym.loc = loc;
	new_sym.i_type = I_VAR;
	new_sym.name = (char*)(malloc(sizeof(char)*(strlen(symbol)+1)));
	new_sym.index = NSYMBOLS-1-nGlobals;
	strcpy(new_sym.name, symbol);
	symtable[NSYMBOLS-1-nGlobals] = new_sym;
	nGlobals++;

	return loc;
}

struct symtableloc sym_process_symbol(char* symbol) {
	struct symtableloc sloc;
	struct symbol s = search_symbol(symbol);

	if (s.i_type != I_NONE) {
		sloc.index = s.index;
		sloc.type = S_RES;
		return sloc;	
	}

	sloc.index = nUnresolved;
	sloc.type = S_UNRES;

	unresolved_symbols[nUnresolved].sym = (char*)(malloc(sizeof(char)*(strlen(symbol)+1)));
	strcpy(unresolved_symbols[nUnresolved].sym, symbol);
	unresolved_symbols[nUnresolved].index = nUnresolved;

	nUnresolved++;
	
	return sloc;
}

char* sym_get_symbol_for(struct symtableloc sloc) {
	if (sloc.type == S_RES)
		return symtable[sloc.index].name;

	for (int i = 0; i < nUnresolved; i++) {
		if (unresolved_symbols[i].index == sloc.index) return unresolved_symbols[i].sym;
	}

	printf("Fatal error in sym_get_symbol_for\n");
	exit(1);
}

struct location sym_declare_symbol(struct symtableloc sloc) {
	if (sloc.type == S_RES) {
		printf("Invalid redeclaration of symbol %s on line %d\n", sym_get_symbol_for(sloc), get_lineNo());
		exit(1);
	}

	char* new_symbol = sym_get_symbol_for(sloc); //unresolved_symbols[sloc.index];
	struct location new_loc = add_symbol(new_symbol);
	nUnresolved--;

	free(new_symbol);

	bool encountered_delete = false;

	for (int i = 0; i < nUnresolved; i++) {
		if (unresolved_symbols[i].index == sloc.index) encountered_delete = true;

		if (encountered_delete) {
			unresolved_symbols[i] = unresolved_symbols[i+1];
		}
	}

	return new_loc;
}
