#include "stmt_eval.h"
#include "tokenize.h"
#include "sym.h"
#include "c8.h"
#include "sym.h"
#include <stdlib.h>


void gen_arith_res(struct ast_node* stmt, int* output) {
	//accumulate what we know - optimization
	//generate code to place what we need in a return value register r0
	//each time we resolve it - we add code to add that result to our accumulatino
	int prevAccumulatedTotal = 0;
	int numResolved = 0;

	for (int i = 0; i < stmt->kChildren; i++) {
		if (stmt->children[i]->t_operatortype == T_INTLIT) {
			numResolved++;
			if (stmt->t_operatortype == T_PLUS || (stmt->t_operatortype == T_MINUS && i == 0)) prevAccumulatedTotal += stmt->children[i]->key.val;
			else if (stmt->t_operatortype == T_MINUS) prevAccumulatedTotal -= stmt->children[i]->key.val;
		}
	}

	int accum_reg = c8_alloc_reg();
	c8_load_instr_const(prevAccumulatedTotal, accum_reg);

	for (int i = 0; i < stmt->kChildren; i++) {
		if (stmt->children[i]->sType == S_FUNC) {
			st_execute(stmt->children[i]);

			if (stmt->t_operatortype == T_PLUS || (stmt->t_operatortype == T_MINUS && i == 0)) {
				c8_add_instr_reg(accum_reg, 0);
			
			} else if (stmt->t_operatortype == T_MINUS) {
				c8_sub_instr_reg(accum_reg, 0);
			
			}

		} else if (stmt->children[i]->t_operatortype == T_IDENT) {
			struct symbol nextS = resolve_gsymbol(stmt->children[i]->key.symbol);
			if (nextS.loc.type == L_REG) {
				if (stmt->t_operatortype == T_PLUS || (stmt->t_operatortype == T_MINUS && i == 0)) {
					c8_add_instr_reg(accum_reg, nextS.loc.loc);
				} else if (stmt->t_operatortype == T_MINUS) {
					c8_sub_instr_reg(accum_reg, nextS.loc.loc);
				}

			} else {
				printf("Memory access error arithmetic\n");
				exit(1);
			}

		} else if (stmt->children[i]->t_operatortype != T_INTLIT) {
			printf("Fatal error\n");	
			exit(1);
		}
	}

	c8_load_instr_reg(0, accum_reg);
}

void execute_setq(struct ast_node* stmt) {
	struct location loc = add_gsymbol(stmt->children[0]->key.symbol);

	if (loc.type == L_REG && stmt->children[1]->t_operatortype == T_IDENT) {
		struct symbol nextS = resolve_gsymbol(stmt->children[1]->key.symbol);

		if (nextS.loc.type == L_REG) {
			c8_load_instr_reg(loc.loc, nextS.loc.loc);
		} else {
			printf("Fatal setq\n");
			exit(1);
		}
	} else if (loc.type == L_REG && stmt->children[1]->t_operatortype == T_INTLIT) {
		c8_load_instr_const(stmt->children[1]->key.val, loc.loc);
	} else if (loc.type == L_REG) {
		st_execute(stmt->children[1]);
		c8_load_instr_reg(loc.loc, 0);
	}
}

void execute_fxn(struct ast_node* stmt) {
	if (stmt->kChildren > 3) {
		printf("Error: functions may not have more than 3 parameters\n");
		exit(1);
	}

	int nRegisters = 0;
	int alloc_registers[3];

	for (int i = 0; i < stmt->kChildren; i++) {
		int next_reg = c8_alloc_reg();
		alloc_registers[i] = next_reg;
		nRegisters++;

		//TODO: change S_FUNC to work with i_type member of resolve_symbol
		if (stmt->children[i]->sType == S_FUNC) {
			st_execute(stmt->children[i]);
			c8_load_instr_reg(next_reg, 0);
		} else if (stmt->children[i]->t_operatortype == T_INTLIT) {
			c8_load_instr_const(stmt->children[i]->key.val, next_reg);
		} else if (stmt->children[i]->t_operatortype == T_IDENT) {
			struct symbol nextS = resolve_gsymbol(stmt->children[i]->key.symbol);
			if (nextS.loc.type == L_REG)
				c8_load_instr_reg(next_reg, nextS.loc.loc);
			else {
				printf("Unsupported memory access (testing)\n");
				exit(1);
			}
		}
	}

	c8_callq(stmt->key.symbol);
	for (int i = 0; i < nRegisters; i++) {
		c8_free_reg(alloc_registers[i]);
	}
}

//this fxn will generate assembly
void st_execute(struct ast_node* stmt) {
	struct ast_node* glue;

	switch (stmt->t_operatortype) {
		case T_IDENT:
			execute_fxn(stmt);
			break;
		case T_INTLIT:
			break;
		case T_PLUS:
		case T_MINUS:
			gen_arith_res(stmt, NULL);
			break;
		case T_SETQ:
			execute_setq(stmt);
			break;
		default:
			printf("Fatal: unsupported statement\n");
			exit(1);
	}

}
