#ifndef C8_H
#define C8_H

#include <stdbool.h>

void c8_init(const char*);

int c8_alloc_reg();
int c8_alloc_param_reg();
void c8_free_reg(int);
void c8_free_allreg();


void c8_cls();
void c8_add_instr_const(int, int);
void c8_add_instr_reg(int, int);
void c8_sub_instr_const(int, int);
void c8_sub_instr_reg(int, int);
void c8_load_instr_const(int, int);
void c8_load_instr_reg(int, int);
void c8_callq(const char*);
void c8_if_stmt_branch(bool, char*, char*);
void c8_jp_label(const char*);
void c8_print_label(const char*);
void c8_eq_req(int, int);
void c8_lt_reg(int, int);
void c8_lt_const(int, int);
void c8_lte_reg(int, int);
void c8_lte_const(int, int);


/*
(+ 1 2 a 3)

(+ 6 a)


 * */
#endif
