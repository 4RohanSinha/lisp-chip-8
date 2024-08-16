#include "stmt_eval.h"
#include "tokenize.h"
#include "sym.h"
#include "c8.h"
#include "sym.h"
#include <stdlib.h>
#include <stdbool.h>


/*
(+ (- 4 3) 2) => this should resolve completely
prevAccumulatedTotal = 2
now we look at (- 4 3)
prevAccumulatedTotal = 1 - then we return and it gets added.

what should happen

(+ (- 4 3 x) a 2)
we look at a particular expression and try to resolve it as much.
	if we can resolve it, we return true, and don't need to look at it again - we can just dump it into an accumulation
	if we can't resolve it, we dump an accumulated total and the other thing in a register, return false - which means that anything depending on it can't be resolved
 **/

bool resolve_exp(struct ast_node* stmt, int* res) {
	int accumulatedTotal = 0;
	bool canResolve = true;
	int accumReg = -1;
	int extraSubReg = -1;
	int innerExp = 0;

	for (int i = 0; i < stmt->kChildren; i++) {
		
		if (stmt->children[i]->t_operatortype == T_INTLIT) {
		
			if (stmt->t_operatortype == T_PLUS || (stmt->t_operatortype == T_MINUS && i == 0)) accumulatedTotal += stmt->children[i]->key.val;
			else if (stmt->t_operatortype == T_MINUS) accumulatedTotal -= stmt->children[i]->key.val;
		
		} else if (stmt->children[i]->t_operatortype == T_PLUS || stmt->children[i]->t_operatortype == T_MINUS) {
		
			canResolve = canResolve && resolve_exp(stmt->children[i], &innerExp);
			if (!canResolve) {

				if (accumReg == -1 && (stmt->t_operatortype == T_PLUS || (stmt->t_operatortype == T_MINUS && i == 0))) {
					accumReg = c8_alloc_reg();
					c8_load_instr_reg(accumReg, 0);
				} else if (extraSubReg == -1 && stmt->t_operatortype == T_MINUS) {
					extraSubReg = c8_alloc_reg();
					c8_load_instr_reg(extraSubReg, 0);
				} else if (stmt->t_operatortype == T_PLUS) c8_add_instr_reg(accumReg, 0);
				else if (stmt->t_operatortype == T_MINUS) c8_add_instr_reg(extraSubReg, 0);

			} else {
				if (stmt->t_operatortype == T_PLUS || (stmt->t_operatortype == T_MINUS && i == 0)) accumulatedTotal += innerExp;
				else if (stmt->t_operatortype == T_MINUS) accumulatedTotal -= innerExp;

			}
		
		} else if (stmt->children[i]->sType == S_FUNC) {
			canResolve = false;
			st_execute(stmt->children[i]);
			if (accumReg == -1 && (stmt->t_operatortype == T_PLUS || (stmt->t_operatortype == T_MINUS && i == 0))) {
				accumReg = c8_alloc_reg();
				c8_load_instr_reg(accumReg, 0);
			} else if (extraSubReg == -1 && stmt->t_operatortype == T_MINUS) {
				extraSubReg = c8_alloc_reg();
				c8_load_instr_reg(extraSubReg, 0);
			
			} else if (stmt->t_operatortype == T_PLUS) c8_add_instr_reg(accumReg, 0);
			else if (stmt->t_operatortype == T_MINUS) c8_add_instr_reg(extraSubReg, 0);

		} else if (stmt->children[i]->t_operatortype == T_IDENT) {
			canResolve = false;
			struct symbol nextS = resolve_symbol(stmt->children[i]->key.sloc);

			if (nextS.loc.type == L_REG) {
				if (accumReg == -1 && (stmt->t_operatortype == T_PLUS || (stmt->t_operatortype == T_MINUS && i == 0))) {
					accumReg = c8_alloc_reg();
					//(- 7 x) vs // (- x 7)
					
					//if we encounter a variable as the first thing, we want to load it into the accumulator
					c8_load_instr_reg(accumReg, nextS.loc.loc);
					//if we encounter a variable as a later operand
				} else if (extraSubReg == -1 && stmt->t_operatortype == T_MINUS) {
					extraSubReg = c8_alloc_reg();
					c8_load_instr_reg(extraSubReg, nextS.loc.loc);
				} else if (stmt->t_operatortype == T_PLUS) {
					c8_add_instr_reg(accumReg, nextS.loc.loc);
				} else if (stmt->t_operatortype == T_MINUS) {
					c8_add_instr_reg(extraSubReg, nextS.loc.loc);
				}

			} else {
				printf("Memory access error arithmetic\n");
				exit(1);
			}
		
		} 
	}

	if (accumulatedTotal > 0 && accumReg != -1) c8_add_instr_const(accumulatedTotal, accumReg);

	if (extraSubReg != -1 && accumReg == -1) {
		accumReg = c8_alloc_reg();
		c8_load_instr_const(accumulatedTotal, accumReg);
	}

	if (extraSubReg != -1) {
		c8_sub_instr_reg(accumReg, extraSubReg);
		c8_free_reg(extraSubReg);
	}

	//accumReg has the final result, or accumulatedTotal
	if (accumReg != -1) {
		c8_load_instr_reg(0, accumReg);
		c8_free_reg(accumReg);
	}
	


	if (canResolve) {
		*res = accumulatedTotal;
	}

	

	return canResolve;
}

void gen_arith_res(struct ast_node* stmt) {
	int x = -1;
	bool didResolve = resolve_exp(stmt, &x);

	if (didResolve) c8_load_instr_const(x, 0);

}
/*
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
			struct symbol nextS = resolve_symbol(stmt->children[i]->key.sloc);
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
*/

void execute_setq(struct ast_node* stmt) {
	struct location loc;
	struct symbol cur_symbol;
	struct symbol next_sym;

	if (stmt->kChildren % 2 == 1) {
		printf("Error: line %d: setq may not have an odd # of parameters\n", get_lineNo());
		exit(1);
	}

	for (int i = 0; i < stmt->kChildren; i+=2) {
		cur_symbol = resolve_symbol(stmt->children[i]->key.sloc);

		if (cur_symbol.i_type == I_NONE) {
			loc = sym_declare_symbol(stmt->children[i]->key.sloc);
		} else {
			loc = cur_symbol.loc;
		}

		if (loc.type == L_REG && stmt->children[i+1]->t_operatortype == T_IDENT && stmt->children[i+1]->sType != S_FUNC) {
			next_sym = resolve_symbol(stmt->children[i+1]->key.sloc);

			if (next_sym.loc.type == L_REG) {
				c8_load_instr_reg(loc.loc, next_sym.loc.loc);
			} else {
				printf("Fatal setq\n");
				exit(1);
			}
		} else if (loc.type == L_REG && stmt->children[i+1]->t_operatortype == T_INTLIT) {
			c8_load_instr_const(stmt->children[i+1]->key.val, loc.loc);
		} else if (loc.type == L_REG) {
			st_execute(stmt->children[i+1]);
			c8_load_instr_reg(loc.loc, 0);
		}
	}
}

void execute_fxn(struct ast_node* stmt) {
	if (stmt->kChildren > 5) {
		printf("Error: functions may not have more than 3 parameters\n");
		exit(1);
	}

	int nRegisters = 0;
	int alloc_registers[5];

	for (int i = 0; i < stmt->kChildren; i++) {
		int next_reg;

		if (stmt->children[i]->kChildren > 0) st_execute(stmt->children[i]);

		next_reg = c8_alloc_param_reg();
		alloc_registers[i] = next_reg;
		nRegisters++;

		if (stmt->children[i]->kChildren > 0) {
			c8_load_instr_reg(next_reg, 0);
		} else if (stmt->children[i]->t_operatortype == T_INTLIT) {
			c8_load_instr_const(stmt->children[i]->key.val, next_reg);
		} else if (stmt->children[i]->t_operatortype == T_IDENT) {
			struct symbol nextS = resolve_symbol(stmt->children[i]->key.sloc);
			if (nextS.loc.type == L_REG)
				c8_load_instr_reg(next_reg, nextS.loc.loc);
			else {
				printf("Unsupported memory access (testing)\n");
				exit(1);
			}
		}
	}

	c8_callq(sym_get_symbol_for(stmt->key.sloc));
	for (int i = 0; i < nRegisters; i++) {
		c8_free_reg(alloc_registers[i]);
	}
}

void execute_cls(struct ast_node* stmt) {
	if (stmt->kChildren > 0) {
		printf("cls received unexpected # of arguments: line %d\n", get_lineNo());
		exit(1);
	}

	c8_cls();
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
			gen_arith_res(stmt);
			break;
		case T_SETQ:
			execute_setq(stmt);
			break;
		case T_CLS:
			execute_cls(stmt);
			break;
		default:
			printf("Fatal: unsupported statement\n");
			exit(1);
	}

}
