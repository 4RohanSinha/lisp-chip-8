#ifndef SYM_H
#define SYM_H

#define NSYMBOLS 1024

enum { L_REG, L_MEM, L_NONE };

struct location {
	int type;
	unsigned int loc;
};

struct symtable {
	char* name;
	struct location loc;
};

extern struct symtable globals[NSYMBOLS];

struct location resolve_gsymbol(char*);
struct location add_gsymbol(char*);

#endif
