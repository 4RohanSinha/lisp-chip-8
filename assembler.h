#ifndef C8_ASSEMBLER_H
#define C8_ASSEMBLER_H

enum { AT_LD, AT_ADD, AT_SUB, AT_CALL, AT_CLS, AT_RET, AT_JP, AT_SNE,
	AT_PRINTDB, AT_PRINTS, AT_PRDRW};
enum { P_REG, P_INTLIT, P_INDEX, P_INDEX_ACC, P_DT, P_ST, P_BCD, P_FSPRITE, P_KEY, P_NONE } ;
enum { DT_INSTR, DT_VAL, DT_REG, DT_SPARAM } ;

struct label {
	char name[10];
	unsigned int addr;
};

struct instr_param {
	int p_type;
	int val;
};

struct instruction {
	int a_type;
	struct instr_param params[3];
	int nParams;
};

struct instr_dec {
	int d_type;
	int a_type;
	int p_type;
	int val;
};

void assembler_init(const char* fname);
void generate_rom();

#endif
