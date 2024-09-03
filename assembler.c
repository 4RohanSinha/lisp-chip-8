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

static void illegal_instr_exit() {
	printf("Abort: Illegal instruction on line %d\n", lineNo);
	exit(1);
}

unsigned int getAddressForLabel(const char* name) {
	for (int i = 0; i < kLabelCount; i++)
		if (!strcmp(label_table[i].name, name)) return label_table[i].addr;
	return -1;
}

static bool jumpToLine(int lineNo) {
	char c;
	int curLine = 1;
	fseek(in_handle, 0, SEEK_SET);
	while (curLine < lineNo && (c = fgetc(in_handle)) != EOF) {
		if (c == '\n') curLine++;
	}

	return curLine == lineNo;
}

void moveAddressesAfterBy(int startingAddr, int offset) {
	for (int i = 0; i < kLabelCount; i++)
		if (label_table[i].addr >= startingAddr) label_table[i].addr += offset;
}

void write_instruction(struct instruction instr) {
	char buffer[2];
	switch (instr.a_type) {
		case AT_CLS:
			if (instr.nParams > 0) illegal_instr_exit();
			buffer[0] = 0x00;
			buffer[1] = 0xe0;
			break;
		case AT_RET:
			if (instr.nParams > 0) illegal_instr_exit();
			buffer[0] = 0x00;
			buffer[1] = 0xee;
			break;
		case AT_JP:
			if (instr.nParams > 1) illegal_instr_exit();
			if (instr.params[0].p_type != P_INTLIT) illegal_instr_exit();
			buffer[0] = (0x1 << 4) + ((instr.params[0].val & 0xf00) >> 8);
			buffer[1] = instr.params[0].val & 0xff;
			break;
		case AT_CALL:
			if (instr.nParams > 1) illegal_instr_exit();
			if (instr.params[0].p_type != P_INTLIT) illegal_instr_exit();
			buffer[0] = (0x2 << 4) + ((instr.params[0].val & 0xf00) >> 8);
			buffer[1] = instr.params[0].val & 0xff;
			break;
		case AT_SHL:
			if (instr.nParams > 1) illegal_instr_exit();
			if (instr.params[0].p_type != P_REG) illegal_instr_exit();
			buffer[0] = (0x8 << 4) + instr.params[0].val;
			buffer[1] = 0x0e;
			break;
		case AT_SE:
			if (instr.nParams > 2) illegal_instr_exit();
			if (instr.params[0].p_type != P_REG) illegal_instr_exit();
			if (instr.params[1].p_type == P_INTLIT) {
				buffer[0] = (0x3 << 4) + (instr.params[0].val);
				buffer[1] = instr.params[1].val & 0xff;
				break;	
			} else if (instr.params[1].p_type == P_REG) {
				buffer[0] = (0x9 << 5) + (instr.params[0].val);
				buffer[1] = (instr.params[1].val << 4) + 0;
				break;
			}

			illegal_instr_exit();
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
			if (instr.nParams == 3 && instr.params[0].p_type == P_REG && instr.params[1].p_type == P_REG && instr.params[2].p_type == P_INTLIT) {

				struct instruction p;
				p.a_type = AT_LD;
				p.nParams = 2;
				p.params[0].p_type = P_REG;
				p.params[0].val = instr.params[0].val;
				p.params[1].p_type = P_INTLIT;
				p.params[1].val = ((instr.params[2].val >> 8) & 0xff);
				write_instruction(p);
				p.params[0].p_type = P_REG;
				p.params[0].val = instr.params[1].val;
				p.params[1].p_type = P_INTLIT;
				p.params[1].val = (instr.params[2].val) & 0xff;
				write_instruction(p);
			} else if (instr.nParams > 2) {
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
				buffer[0] = (0xA << 4) + ((instr.params[1].val >> 8) & 0xf);
				buffer[1] = (instr.params[1].val) & 0xff;

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
		
		case AT_PRINTDB:
			if (instr.nParams > 1) illegal_instr_exit();
			
			if (instr.params[0].p_type == P_INDEX) {
				buffer[0] = 0x00;
				buffer[1] = 0xea;
			} else if (instr.params[0].p_type == P_REG) {
				buffer[0] = (0xF << 4) + instr.params[0].val;
				buffer[1] = 0x95;
			} else if (instr.params[0].p_type == P_INDEX_ACC) {
				buffer[0] = 0x00;
				buffer[1] = 0xeb;
			} else if (instr.params[0].p_type == P_INTLIT) {
				buffer[0] = (0x8 << 4) + (instr.params[0].val & 0xF0);
				buffer[1] = ((instr.params[0].val & 0xF) << 4) + 0xA;
			} else { illegal_instr_exit(); }
			break;
		case AT_PRINTS:
			if (instr.nParams > 1) illegal_instr_exit();
			if (instr.params[0].p_type != P_INDEX) illegal_instr_exit();

			buffer[0] = 0x00;
			buffer[1] = 0xec;
			break;

		case AT_PRDRW:
			if (instr.nParams != 2) illegal_instr_exit();

			if (instr.params[0].p_type == P_INDEX && instr.params[1].p_type == P_INTLIT) {
				buffer[0] = (0xF << 4) + (instr.params[1].val & 0xF);
				buffer[1] = 0x97;
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

	id.a_type = -1;

	switch (*text) {
		case 'a':
			if (!strcmp("add", text)) id.a_type = AT_ADD;
		case 'c':
			if (!strcmp("call", text)) id.a_type = AT_CALL;
			if (!strcmp("cls", text)) id.a_type = AT_CLS;
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
		case 'j':
			if (!strcmp("jp", text)) id.a_type = AT_JP;
		case 'p':
			if (!strcmp("printdb", text)) id.a_type = AT_PRINTDB;
			if (!strcmp("prints", text)) id.a_type = AT_PRINTS;
			if (!strcmp("prdrw", text)) id.a_type = AT_PRDRW;
		case 'r':
			if (!strcmp("ret", text)) id.a_type = AT_RET;
		case 's':
			if (!strcmp("sub", text)) id.a_type = AT_SUB;
			if (!strcmp("sne", text)) id.a_type = AT_SNE;
			if (!strcmp("se", text)) id.a_type = AT_SE;
			if (!strcmp("shl", text)) id.a_type = AT_SHL;
		default:
			if (id.a_type != -1) break;
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
		case DT_SPARAM:
			instr->params[(instr->nParams)++].p_type = decoding->p_type;
			break;
		default:
			illegal_instr_exit();
	}
}

void update_label_table(const char* lbl, int addr) {
	strcpy(label_table[kLabelCount++].name, lbl);
	label_table[kLabelCount-1].addr = addr;
}

int process_data_string() {
	bool encounteredOpenQuote = false;
	char c;
	int size = 0;

	while ((c = fgetc(in_handle)) != EOF) {
		if (encounteredOpenQuote) {
			if (c == '\n') lineNo++;
			if (c == '"') {
				fputc(0, out_handle);
				size++;
				return size;
			} else {
				fputc(c, out_handle);
				size++;
			}
		} else if (c == '"') encounteredOpenQuote = true;
	}

	illegal_instr_exit();
}

int process_labels() {
	char c;
	char text_buffer[20];
	int i = 0;
	int numWords = 0;
	bool labelAddedOnCurLine = false;
	int endOfData = 0x200;
	int curInstruction = 0x200;
	int newDataSize = 0;
	bool movedFp = false;
	while ((c = fgetc(in_handle)) != EOF) {
		if (i >= 19) illegal_instr_exit();
		
		if (c == ':') {
			if (numWords > 0) illegal_instr_exit();
			text_buffer[i] = '\0';
			if (strstr(text_buffer, ".str") != NULL) {
				if (!movedFp) { fseek(out_handle, 2, SEEK_CUR); movedFp = true; }
				newDataSize = process_data_string();
				moveAddressesAfterBy(endOfData, newDataSize);
				update_label_table(text_buffer, endOfData);
				endOfData += newDataSize;
				curInstruction += newDataSize;
			} else {
				update_label_table(text_buffer, curInstruction);
			}
			/*strcpy(label_table[kLabelCount++].name, text_buffer);
			label_table[kLabelCount-1].addr = pc;*/
			i = 0;
			labelAddedOnCurLine = true;
		} else if (isalpha(c) || isdigit(c) || c == '_' || c == '.') text_buffer[i++] = c;
		else if (isspace(c)) {
			text_buffer[i] = '\0';
			if (i != 0) numWords++;
			i = 0;
		} if (c == '\n') {
			lineNo++;
			if (numWords > 0 && !labelAddedOnCurLine) curInstruction += 2;
			numWords = 0;
			labelAddedOnCurLine = false;
		}
	}

	return endOfData;
}

void generate_rom() {
	char c;
	char text_buffer[20];
	struct instruction cur_instruction;
	struct instr_dec decoding;
	bool skipCurLine = false;
	bool convertStr = false;
	bool openingQuoteEncountered = false;
	int i = 0;
	int mainAddr;

	cur_instruction.a_type = -1;
	cur_instruction.nParams = 0;

	process_labels();

	lineNo = 1;

	mainAddr = getAddressForLabel("main");

	if (mainAddr != 0x200) {
		moveAddressesAfterBy(0x200, 2);
		mainAddr = getAddressForLabel("main");
	}


	if (mainAddr == -1) {
		printf("Fatal: no entry point\n");
		exit(1);
	} else if (mainAddr != 0x200) {
		cur_instruction.a_type = AT_JP;
		cur_instruction.nParams = 1;
		cur_instruction.params[0].p_type = P_INTLIT;
		cur_instruction.params[0].val = mainAddr;
		fseek(out_handle, 0, SEEK_SET);
		write_instruction(cur_instruction);
		fseek(out_handle, 0, SEEK_END);
		cur_instruction.a_type = -1;
		cur_instruction.nParams = 0;
	}
	
	fseek(in_handle, 0, SEEK_SET);

	while ((c = fgetc(in_handle)) != EOF) {
		if (i >= 19) {
			illegal_instr_exit();
		}

		if ((c == ',' || c == ' ' || c == '\n') && i > 0) {
			text_buffer[i] = '\0';
			if (!strcmp(text_buffer, ".str")) skipCurLine = true;
			if (!skipCurLine) {
				decoding = decode(text_buffer);
				update_instruction(&cur_instruction, &decoding);
			}
			i = 0;
		} else if (c == ':') {
			skipCurLine = true;
			text_buffer[i] = '\0';
			convertStr = (strstr(text_buffer, ".str") != NULL);
		} else if (!isspace(c) && !skipCurLine) {
			text_buffer[i++] = c;
		}
		
		if (c == '\n') {
			if (cur_instruction.a_type != -1 && !skipCurLine) {
				write_instruction(cur_instruction);
				cur_instruction.a_type = -1;
				cur_instruction.nParams = 0;
			}
			lineNo++;

			i = 0;
			skipCurLine = false;
		}
		
	}
}

int main(int argc, char** argv) {
	assembler_init(argv[1]);
	generate_rom();
	kLabelCount = 0;
}
