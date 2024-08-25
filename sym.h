#ifndef SYM_H
#define SYM_H

#define NSYMBOLS 1024

enum { L_REG, L_MEM, L_NONE, };

enum { I_FUNC, I_VAR, I_NONE };

enum { S_RES, S_UNRES };

enum { M_SPRITE, M_STRING };

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

struct m_object_loc {
	int m_type;
	int id;
	char* data;
	char symbol[100];
};

struct symbol resolve_symbol(struct symtableloc);
struct symtableloc sym_process_symbol(char*);
struct symbol sym_declare_symbol(struct symtableloc);
char* sym_get_symbol_for(struct symtableloc);
char* sym_get_symbol_by_loc(struct location);

int m_object_add_data(char*, int);
void m_object_rename_data(char*, char*);
char* m_object_get_label_for_index(int);
void m_object_asm_dump();
#endif
