#ifndef C8_H
#define C8_H

void c8_init(const char*);

int c8_alloc_reg();
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
/*
(+ 1 2 a 3)

(+ 6 a)


 * */
#endif
