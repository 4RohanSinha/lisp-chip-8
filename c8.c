#include "c8.h"
#include <stdlib.h>
#include <stdio.h>

FILE* out_file;

static int reg_use[16];

void c8_init(const char* fname) {
	out_file = fopen(fname, "w");
}

//r0 - return addr
int c8_alloc_reg() {
	for (int i = 5; i < 16; i++) {
		if (reg_use[i] == 0) {
			reg_use[i] = 1;
			return i;
		}	
	}

	return -1;
}

void c8_free_reg(int reg) {
	reg_use[reg] = 0;
}

void c8_free_allreg() {
	for (int i = 0; i < 16; i++) reg_use[i] = 0;
}

void c8_cls() {
	fprintf(out_file, "\tcls\n");
}

void c8_add_instr_const(int c, int reg) {
	char x = c & 0xff;
	fprintf(out_file, "\tadd V%d, %d\n", reg, x);
}

void c8_sub_instr_const(int c, int reg) {
	char x = c & 0xff;
	int r1 = c8_alloc_reg();
	fprintf(out_file, "\tld V%d, %d\n\tsub V%d, V%d\n", r1, x, r1, reg);
	reg_use[reg] = 1;
	c8_free_reg(r1);
}

void c8_sub_instr_reg(int dest, int src) {
	fprintf(out_file, "\tsub V%d, V%d\n", dest, src);
	reg_use[dest] = 1;
}

void c8_load_instr_const(int c, int reg) {
	char x = c & 0xff;
	fprintf(out_file, "\tld V%d, %d\n", reg, x);
	reg_use[reg] = 1;
}

void c8_load_instr_reg(int dest, int src) {
	fprintf(out_file, "\tld V%d, V%d\n", dest, src);
	reg_use[dest] = 1;
	c8_free_reg(src);
}

void c8_add_instr_reg(int dest, int src) {
	fprintf(out_file, "\tadd V%d, V%d\n", dest, src);
	reg_use[dest] = 1;
}
