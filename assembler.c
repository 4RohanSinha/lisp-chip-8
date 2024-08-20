#include "assembler.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>

#define INSTR_LENGTH 5

static FILE* in_handle;
static FILE* out_handle;
static int lineNo;
static struct label label_table[1024];
static int kLabelCount;
int pc = 0x200;

static void illegal_instr_exit() {
	printf("Abort: Illegal instruction on line %d\n", lineNo);
	exit(1);
}

unsigned int getAddressForLabel(const char* name) {
	for (int i = 0; i < kLabelCount; i++)
		if (!strcmp(label_table[i].name, name)) return label_table[i].addr;
	return -1;
}

void moveAddressesAfterBy(int startingAddr, int offset) {
	for (int i = 0; i < kLabelCount; i++)
		if (label_table[i].addr > startingAddr) label_table[i].addr += offset;
}

void write_instruction(struct instruction instr) {
	char buffer[2];

	switch (instr.a_type) {
		case AT_CLS:
			if (instr.nParams > 0) illegal_instr_exit();
			buffer[0] = 0x00;
			buffer[1] = 0xe0;
			break;
		case AT_JP:
			if (instr.nParams > 1) illegal_instr_exit();
			if (instr.params[0].p_type != P_INTLIT) illegal_instr_exit();
			//	printf("%x\n", getAddressForLabel(label_table[kLabelCount-1].name));
			//printf("%x\n", instr.params[0].val);
			buffer[0] = (0x1 << 4) + ((instr.params[0].val & 0xf00) >> 8);
			buffer[1] = instr.params[0].val & 0xff;
			break;
		case AT_SNE:
			if (instr.nParams > 2) illegal_instr_exit();
			if (instr.params[0].p_type != P_REG) illegal_instr_exit();
			if (instr.params[1].p_type == P_INTLIT) {
				buffer[0] = (0x4 << 4) + (instr.params[0].val);
				buffer[1] = instr.params[1].val & 0xff;
				break;	
			} else if (instr.params[1].p_type == P_REG) {
				buffer[0] = (0x9 << 4) + (instr.params[0].val);
				buffer[1] = (instr.params[1].val << 4) + 0;
				break;
			}

			illegal_instr_exit();
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
		case 'j':
			if (!strcmp("jp", text)) id.a_type = AT_JP;
			break;
		case 'r':
			if (!strcmp("ret", text)) id.a_type = AT_RET;
			break;
		case 's':
			if (!strcmp("sub", text)) id.a_type = AT_SUB;
			if (!strcmp("sne", text)) id.a_type = AT_SNE;
			break;
		default:
			if (getAddressForLabel(text) != -1) {
				id.d_type = DT_VAL;
				id.p_type = P_INTLIT;
				id.val = getAddressForLabel(text);
				return id;
			}

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

void process_labels() {
	char c;
	char text_buffer[10];
	int i = 0;
	int numWords = 0;
	bool labelAddedOnCurLine = false;

	while ((c = fgetc(in_handle)) != EOF) {
		if (i >= 9) illegal_instr_exit();
		
		if (c == ':') {
			if (numWords > 0) illegal_instr_exit();
			text_buffer[i] = '\0';
			strcpy(label_table[kLabelCount++].name, text_buffer);
			label_table[kLabelCount-1].addr = pc;
			i = 0;
			labelAddedOnCurLine = true;
		} else if (isalpha(c) || isdigit(c) || c == '_') text_buffer[i++] = c;
		else if (isspace(c)) {
			i = 0;
			numWords++;
		} if (c == '\n') {
			lineNo++;
			numWords = 0;
			if (!labelAddedOnCurLine) pc += 2;
			labelAddedOnCurLine = false;
		}
	}
}

void generate_rom() {
	char c;
	char text_buffer[10];
	struct instruction cur_instruction;
	struct instr_dec decoding;
	bool skipCurLine = false;
	int i = 0;

	cur_instruction.a_type = -1;
	cur_instruction.nParams = 0;

	process_labels();

	fseek(in_handle, 0, SEEK_SET);
	lineNo = 0;

	if (getAddressForLabel("main") == -1) {
		printf("Fatal: no entry point\n");
		exit(1);
	}

	while ((c = fgetc(in_handle)) != EOF) {
		if (i >= 9) {
			illegal_instr_exit();
		}

		if ((c == ',' || c == ' ' || c == '\n') && i > 0) {
			if (!skipCurLine) {
				text_buffer[i] = '\0';
				decoding = decode(text_buffer);
				update_instruction(&cur_instruction, &decoding);
			}
			i = 0;
		} else if (c == ':') {
			skipCurLine = true;
			text_buffer[i] = '\0';
		} else if (!isspace(c)) {
			text_buffer[i++] = c;
		}
		
		if (c == '\n') {
			if (!skipCurLine) {
				write_instruction(cur_instruction);
				lineNo++;
				cur_instruction.a_type = -1;
				cur_instruction.nParams = 0;
			}

			skipCurLine = false;
		}
		
	}
}

int main(int argc, char** argv) {
	assembler_init(argv[1]);
	generate_rom();
	kLabelCount = 0;
}
