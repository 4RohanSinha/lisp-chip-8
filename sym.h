#ifndef SYM_H
#define SYM_H

#define NSYMBOLS 1024

struct symtable {
	char* name;
	unsigned int loc;
};

extern struct symtable globals[NSYMBOLS];

#endif
