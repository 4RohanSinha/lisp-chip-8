#ifndef C8_H
#define C8_H

void c8_init(const char*);

int alloc_reg();
void free_reg(int);
void free_allreg();

void c8_cls();

#endif
