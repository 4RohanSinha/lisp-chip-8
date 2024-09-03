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

typedef int(*accum_val_t)(int, int);
typedef int(*accum_reg_t)(int*, int*, int); //accumulation register, subtraction register, input register

void resolve_exp_to_reg(struct ast_node* stmt, int output_reg) {
	struct symbol nextS;

	switch (stmt->t_operatortype) {
		case T_IDENT:
			if (stmt->sType == S_FUNC) {
				st_execute(stmt);
				if (output_reg != 0) c8_load_instr_reg(output_reg, 0);
				break;
			}

			nextS = resolve_symbol(stmt->key.sloc);
			if (nextS.loc.type == L_REG) {
				c8_load_instr_reg(output_reg, nextS.loc.loc);
			} else {
				printf("Fatal: unknown identifier on line %d\n", get_lineNo());
				exit(1);
			}

			break;
		case T_INTLIT:
			c8_load_instr_const(stmt->key.val, output_reg);
			break;
		case T_STRING:
			c8_load_instr_label(m_object_get_label_for_index(stmt->key.mloc), output_reg);
			break;
		default:
			st_execute(stmt);
			if (output_reg != 0) c8_load_instr_reg(output_reg, 0);
	}
}

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

void execute_setq(struct ast_node* stmt) {
	struct symbol cur_symbol;
	struct symbol next_sym;

	if (stmt->kChildren % 2 == 1) {
		printf("Error: line %d: setq may not have an odd # of parameters\n", get_lineNo());
		exit(1);
	}

	for (int i = 0; i < stmt->kChildren; i+=2) {
		resolve_exp_to_reg(stmt->children[i+1], 0);

		cur_symbol = resolve_symbol(stmt->children[i]->key.sloc);

		if (cur_symbol.i_type == I_NONE) {
			cur_symbol = sym_declare_symbol(stmt->children[i]->key.sloc);
		}

		c8_load_instr_reg(cur_symbol.loc.loc, 0);

	}
}

void execute_fxn(struct ast_node* stmt) {
	if (stmt->kChildren > 5) {
		printf("Error: functions may not have more than 3 parameters\n");
		exit(1);
	}

	int nRegisters = 0;
	int alloc_registers[5];
	int product_registers[5]; //registers that have a function evaluation that should be done first

	//this is to make sure that the function evaluation happens before the function call
	for (int i = 0; i < stmt->kChildren; i++) {
		if (stmt->children[i]->sType == S_FUNC) {
			st_execute(stmt->children[i]);
			product_registers[i] = c8_alloc_reg();
			c8_load_instr_reg(product_registers[i], 0);
		} else {
			product_registers[i] = -1;
		}
	}

	for (int i = 0; i < stmt->kChildren; i++) {
		int next_reg;

		next_reg = c8_alloc_param_reg();
		alloc_registers[i] = next_reg;
		nRegisters++;
		
		if (stmt->children[i]->sType == S_FUNC) {
			c8_load_instr_reg(next_reg, product_registers[i]);
		} else {
			resolve_exp_to_reg(stmt->children[i], next_reg);
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

void execute_if(struct ast_node* stmt) {
	struct symbol nextS;
	char else_label[15];
	char endif_label[15];
	bool hasElse = stmt->kChildren == 3;

	if (stmt->kChildren < 2 || stmt->kChildren > 3) {
		printf("Invalid if statement on line %d\n", get_lineNo());
		exit(1);
	}

	resolve_exp_to_reg(stmt->children[0], 0);

	c8_if_stmt_branch(true, else_label, endif_label);

	resolve_exp_to_reg(stmt->children[1], 0);
	c8_jp_label(endif_label);
	c8_print_label(else_label);

	if (hasElse) {
		resolve_exp_to_reg(stmt->children[2], 0);
	} else {
		c8_load_instr_const(0, 0);	
	}

	c8_print_label(endif_label);


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
		case T_IF:
			execute_if(stmt);
			break;
		default:
			printf("Fatal: unsupported statement\n");
			exit(1);
	}

}
