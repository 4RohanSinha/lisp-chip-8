#include "misc.h"
#include <stdio.h>
#include <stdlib.h>

void syntaxerror(const char* msg) {
	fprintf(stderr, "%s", msg);
	exit(1);
}
