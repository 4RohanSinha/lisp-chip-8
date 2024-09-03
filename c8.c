#include "c8.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

FILE* out_file;

static int reg_use[16];
static int lblCount;

void c8_stdlib_dump() {
	FILE* in_file = fopen("stdlib.asm", "r");
	char c;

	while ((c = fgetc(in_file)) != EOF) {
		fputc(c, out_file);
	}
}
void c8_init(const char* fname) {
	out_file = fopen(fname, "w");
	lblCount = 0;
	c8_stdlib_dump();
	fprintf(out_file, "main:\n");
}

//r0 - return addr
int c8_alloc_reg() {
	for (int i = 7; i < 14; i++) {
		if (reg_use[i] == 0) {
			reg_use[i] = 1;
			return i;
		}	
	}

	printf("Register overflow\n");
	exit(1);
}

int c8_alloc_param_reg() {
	for (int i = 2; i < 7; i++) {
		if (reg_use[i] == 0) {
			reg_use[i] = 1;
			return i;
		}
	}

	printf("Parameter register overflow\n");
	exit(1);
}

void c8_free_reg(int reg) {
	reg_use[reg] = 0;
}

void c8_free_allreg() {
	for (int i = 1; i < 15; i++) reg_use[i] = 0;
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

void c8_load_instr_label(char* lbl, int reg) {
	if (reg_use[reg+1] == 1) {
		printf("Fatal: label load error\n");
		exit(1);
	}

	reg_use[reg+1] = 1;
	fprintf(out_file, "\tld V%d, V%d, %s\n", reg, reg+1, lbl);
	reg_use[reg] = 1;
}

void c8_load_instr_reg(int dest, int src) {
	fprintf(out_file, "\tld V%d, V%d\n", dest, src);
	reg_use[dest] = 1;
	//c8_free_reg(src);
}

void c8_add_instr_reg(int dest, int src) {
	fprintf(out_file, "\tadd V%d, V%d\n", dest, src);
	reg_use[dest] = 1;
}

void c8_callq(const char* method) {
	fprintf(out_file, "\tcall %s\n", method);
}

void c8_if_stmt_branch(bool has_else, char* else_label, char* endif_label) {

	sprintf(endif_label, "LBL_%d", lblCount);
	lblCount++;

	fprintf(out_file, "\tsne V0, 0\n");
	if (has_else) {
		sprintf(else_label, "LBL_%d", lblCount);
		lblCount++;
		c8_jp_label(else_label);
	} else {
		else_label = NULL;
		c8_jp_label(endif_label);
	}
}

void c8_jp_label(const char* label) {
	fprintf(out_file, "\tjp %s\n", label);
}

void c8_print_label(const char* label) {
	fprintf(out_file, "%s:\n", label);
}

void c8_dump_str_label(const char* label, const char* data) {
	fprintf(out_file, "%s:\n", label);
	fprintf(out_file, "\t.str \"%s\"", data);
}
