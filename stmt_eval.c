#include "stmt_eval.h"
#include "tokenize.h"
#include "sym.h"
#include "c8.h"
#include <stdlib.h>

void gen_arith_res(struct ast_node* stmt) {
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
		if (stmt->children[i]->t_operatortype != T_INTLIT) {
			st_execute(stmt->children[i]);

			if (stmt->t_operatortype == T_PLUS || (stmt->t_operatortype == T_MINUS && i == 0)) {
				c8_add_instr_reg(accum_reg, 0);
			
			} else if (stmt->t_operatortype == T_MINUS) {
				c8_sub_instr_reg(accum_reg, 0);
			
			}

		}
	}

	c8_load_instr_reg(0, accum_reg);

}

//this fxn will generate assembly
void st_execute(struct ast_node* stmt) {
	struct ast_node* glue;

	switch (stmt->t_operatortype) {
		case T_INTLIT:
			glue = (struct ast_node*)malloc(sizeof(struct ast_node));
			glue->key.val = stmt->key.val;
			glue->t_operatortype = T_INTLIT;
			glue->kChildren = 0;
			break;
		case T_PLUS:
		case T_MINUS:
			gen_arith_res(stmt);
			break;
		default:
			printf("Fatal: unsupported statement\n");
			exit(1);
	}

}
