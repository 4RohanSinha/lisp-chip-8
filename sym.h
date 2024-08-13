#ifndef SYM_H
#define SYM_H

#define NSYMBOLS 1024

enum { L_REG, L_MEM, L_NONE };

enum { I_FUNC, I_VAR, I_NONE };

struct location {
	int type;
	unsigned int loc;
};

struct symbol {
	char* name;
	struct location loc;
	int i_type;
};


struct symbol resolve_gsymbol(char*);
struct location add_gsymbol(char*);

#endif
