#include "assembler.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#define INSTR_LENGTH 5

static FILE* in_handle;
static FILE* out_handle;
static int lineNo;

static void illegal_instr_exit() {
	printf("Abort: Illegal instruction on line %d\n", lineNo);
	exit(1);
}

void write_instruction(struct instruction instr) {
	char buffer[2];

	switch (instr.a_type) {
		case AT_CLS:
			if (instr.nParams > 0) illegal_instr_exit();
			buffer[0] = 0x00;
			buffer[1] = 0xe0;
			break;
		case AT_ADD:
			if (instr.nParams > 2) {
				illegal_instr_exit();
			}

			//ADD Vx, byte
			if (instr.params[0].p_type == P_REG && instr.params[1].p_type == P_INTLIT) {
				buffer[0] = (0x7 << 4) + (instr.params[0].val);
				buffer[1] = instr.params[1].val;

			//ADD Vx, Vy
			} else if (instr.params[0].p_type == P_REG && instr.params[1].p_type == P_REG) {
				buffer[0] = (0x8 << 4) + instr.params[0].val;
				buffer[1] = (instr.params[1].val << 4) + 0x4;

			//ADD I, Vx
			} else if (instr.params[0].p_type == P_INDEX && instr.params[1].p_type == P_REG) {
				buffer[0] = (0xf << 4) + instr.params[1].val;
				buffer[1] = 0x1e;
			} else {
				illegal_instr_exit();
			}

			break;

		case AT_SUB:
			if (instr.nParams > 2) {
				illegal_instr_exit();
			}

			//SUB Vx, Vy
			if (instr.params[0].p_type == P_REG && instr.params[1].p_type == P_REG) {
				buffer[0] = (0x8 << 4) + instr.params[0].val;
				buffer[1] = (instr.params[1].val << 4) + 0x4;
			} else {
				illegal_instr_exit();
			}

			break;
		case AT_LD:
			if (instr.nParams > 2) {
				illegal_instr_exit();
			}

			//LD Vx, byte
			if (instr.params[0].p_type == P_REG && instr.params[1].p_type == P_INTLIT) {
				buffer[0] = (0x6 << 4) + (instr.params[0].val);
				buffer[1] = instr.params[1].val;

			//LD Vx, Vy
			} else if (instr.params[0].p_type == P_REG && instr.params[1].p_type == P_REG) {
				buffer[0] = (0x8 << 4) + instr.params[0].val;
				buffer[1] = (instr.params[1].val << 4) + 0x0;

			//LD I, addr
			} else if (instr.params[0].p_type == P_INDEX && instr.params[1].p_type == P_INTLIT) {
				buffer[0] = (0xA << 4) + (instr.params[1].val >> 8);
				buffer[1] = (instr.params[1].val >> 4) + (instr.params[1].val);

			//LD [I], Vx
			} else if (instr.params[0].p_type == P_INDEX_ACC && instr.params[1].p_type == P_REG) {
				buffer[0] = (0xf << 4) + instr.params[1].val;
				buffer[1] = 0x55;

			//LD Vx, [I]
			} else if (instr.params[0].p_type == P_REG && instr.params[1].p_type == P_INDEX_ACC) {
				buffer[0] = (0xf << 4) + instr.params[0].val;
				buffer[1] = 0x65;
			} else {
				illegal_instr_exit();
			}

			break;
		default:
			illegal_instr_exit();
	}

	fwrite(buffer, sizeof(char), 2, out_handle);
}

void assembler_init(const char* fname) {
	in_handle = fopen(fname, "r");
	out_handle = fopen("out.rom", "wb");	
	lineNo = 1;

	if (in_handle == NULL || out_handle == NULL) {
		printf("Error opening file %s\n", fname);
		exit(1);
	}
}

static int get_int(char* text) {
	int literal = 0;
	int i = 0;
	while (text[i] != '\0') {
		if (!isdigit(*text)) illegal_instr_exit();
		if (i > 0) literal *= 10;
		literal += (text[i] - '0');
		i++;
	}

	return literal;
}

static int get_hex(char* text) {
//TODO
	return 0;
}

static struct instr_dec decode(char* text) {
	struct instr_dec id;
	int literal = 0;

	int i = 0;

	if (isdigit(*text) && text[1] != 'x') {
		literal = get_int(text);
		id.d_type = DT_VAL;
		id.p_type = P_INTLIT;
		id.val = literal;
		return id;
	}

	if (text[0] == '0' && text[1] == 'x') {

	}

	if (*text == 'V') {
		literal = get_int(text+1);
		id.d_type = DT_REG;
		id.p_type = P_REG;
		id.val = literal;
		return id;
	}

	switch (*text) {
		case 'a':
			if (!strcmp("add", text)) id.a_type = AT_ADD;
			break;
		case 'c':
			if (!strcmp("call", text)) id.a_type = AT_CALL;
			if (!strcmp("cls", text)) id.a_type = AT_CLS;
			break;
		case 'I':
			if (!strcmp("I", text)) {
				id.d_type = DT_SPARAM;
				id.p_type = P_INDEX;
				return id;
			}
		case '[':
			if (!strcmp("[I]", text)) {
				id.d_type = DT_SPARAM;
				id.p_type = P_INDEX_ACC;
				return id;
			}
		case 'l':
			if (!strcmp("ld", text)) id.a_type = AT_LD;
			break;
		case 'r':
			if (!strcmp("ret", text)) id.a_type = AT_RET;
			break;
		case 's':
			if (!strcmp("sub", text)) id.a_type = AT_SUB;
			break;
		default:
			illegal_instr_exit();

	}

	id.d_type = DT_INSTR;
	return id;
}


static void update_instruction(struct instruction* instr, struct instr_dec* decoding) {
	if (instr->nParams == 3) illegal_instr_exit();
	switch (decoding->d_type) {
		case DT_INSTR:
			if (instr->a_type != -1) illegal_instr_exit();
			instr->a_type = decoding->a_type;
			break;
		case DT_VAL:
			instr->params[(instr->nParams)++].p_type = P_INTLIT;
			instr->params[(instr->nParams)-1].val = decoding->val;
			break;
		case DT_REG:
			instr->params[(instr->nParams)++].p_type = P_REG;
			instr->params[(instr->nParams)-1].val = decoding->val;
			break;
		default:
			illegal_instr_exit();
	}
}

void generate_rom() {
	char c;
	char text_buffer[6];
	struct instruction cur_instruction;
	struct instr_dec decoding;
	int i = 0;

	cur_instruction.a_type = -1;
	cur_instruction.nParams = 0;

	while ((c = fgetc(in_handle)) != EOF) {
		if (i >= 5) {
			illegal_instr_exit();
		}

		if ((c == ',' || c == ' ' || c == '\n') && i > 0) {
			text_buffer[i] = '\0';
			decoding = decode(text_buffer);
			update_instruction(&cur_instruction, &decoding);
			i = 0;
		} else if (!isspace(c)) {
			text_buffer[i++] = c;
		}
		
		if (c == '\n') {
			write_instruction(cur_instruction);
			lineNo++;
			cur_instruction.a_type = -1;
			cur_instruction.nParams = 0;
		}
		
	}
}

int main(int argc, char** argv) {
	assembler_init(argv[1]);
	generate_rom();
}
