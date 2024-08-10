#include "misc.h"
#include <stdio.h>
#include <stdlib.h>

void syntaxerror(const char* msg) {
	fprintf(stderr, msg);
	exit(1);
}
