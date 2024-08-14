#ifndef SYM_H
#define SYM_H

#define NSYMBOLS 1024

enum { L_REG, L_MEM, L_NONE, };

enum { I_FUNC, I_VAR, I_NONE };

enum { S_RES, S_UNRES };

struct location {
	int type;
	unsigned int loc;
};

struct symbol {
	char* name;
	struct location loc;
	int i_type;
	int index;
};

struct symtableloc {
	int index;
	int type;
};

struct unresolved_symbol {
	char* sym;
	int index;
};

struct symbol resolve_symbol(struct symtableloc);
struct symtableloc sym_process_symbol(char*);
struct location sym_declare_symbol(struct symtableloc);
char* sym_get_symbol_for(struct symtableloc);
#endif
