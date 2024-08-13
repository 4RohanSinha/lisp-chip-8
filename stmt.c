#include "stmt.h"
#include "stmt_eval.h"
#include "tokenize.h"
#include "ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char* tokenToString(int token) {
    switch (token) {
        case T_OPEN_PAREN: return "T_OPEN_PAREN";
        case T_CLOSE_PAREN: return "T_CLOSE_PAREN";
        case T_PLUS: return "T_PLUS";
        case T_MINUS: return "T_MINUS";
        case T_STAR: return "T_STAR";
        case T_SLASH: return "T_SLASH";
        case T_INTLIT: return "T_INTLIT";
        case T_SETQ: return "T_SETQ";
        case T_LET: return "T_LET";
        case T_IDENT: return "T_IDENT";
        case T_EOF: return "T_EOF";
        default: return "UNKNOWN_TOKEN";
    }
}

static int operators[] = {T_PLUS, T_MINUS, T_STAR, T_SLASH, T_SETQ, T_LET, T_IDENT};
static struct token t;
//each process paren returns an ast_node*! a tree, basically.
static struct ast_node* process_paren() {
	struct ast_node* cur_node = (struct ast_node*)malloc(sizeof(struct ast_node));

	//add operator
	scan(&t);

	cur_node->t_operatortype = t.tokentype;
	cur_node->kChildren = 0;
	
	if (t.tokentype == T_IDENT) {
		cur_node->key.symbol = (char*)(malloc(sizeof(char)*(strlen(t.val.charval)+1)));
		strcpy(cur_node->key.symbol, t.val.charval);
	}

	match_multiple(&t, operators, sizeof(operators)/sizeof(int), "operator");

	cur_node->sType = S_FUNC;


	// add children nodes
	while (t.tokentype != T_CLOSE_PAREN) {
		struct ast_node* new_child;
		//printf("%s\n", tokenToString(t.tokentype));
		switch (t.tokentype) {
			case T_OPEN_PAREN:
				cur_node->children[(cur_node->kChildren)++] = process_paren();
				break;
			case T_CLOSE_PAREN:
				break;
			case T_INTLIT:
				new_child = (struct ast_node*)malloc(sizeof(struct ast_node));
				new_child->key.val = t.val.intval;
				new_child->t_operatortype = T_INTLIT;
				new_child->kChildren = 0;
				cur_node->children[(cur_node->kChildren)++] = new_child;
				break;
			case T_IDENT:
				new_child = (struct ast_node*)malloc(sizeof(struct ast_node));
				new_child->key.symbol = (char*)(malloc(sizeof(char)*(strlen(t.val.charval)+1)));
				strcpy(new_child->key.symbol, t.val.charval);
				new_child->t_operatortype = T_IDENT;
				new_child->kChildren = 0;
				cur_node->children[(cur_node->kChildren)++] = new_child;
				break;
				
			default:
				printf("Illegal token on line %d\n", get_lineNo());
				exit(1);
				
		}
		scan(&t);
	} 

	return cur_node;
}

static void print_ast(struct ast_node* tree, int indent) {
	for (int i = 0; i < indent; i++) printf("\t");
	if (tree->t_operatortype == T_INTLIT) printf("%d ", tree->key.val);
	else if (tree->t_operatortype == T_IDENT) printf("%s ", tree->key.symbol);
	printf("%s %d\n", tokenToString(tree->t_operatortype), tree->kChildren);
	for (int i = 0; i < tree->kChildren; i++)
		print_ast(tree->children[i], indent+1);
}

void st_parse() {
	struct ast_node* stmt = NULL;

	while (scan(&t)) {
		//printf("%s\n", tokenToString(t.tokentype));
		//printf("\n");
		switch (t.tokentype) {
			case T_INTLIT:
				break;
			case T_OPEN_PAREN:
				stmt = process_paren();
				print_ast(stmt, 0);
				st_execute(stmt);
				break;
			default:
				printf("Syntax error: unexpected token on line %d\n", get_lineNo());
				exit(1);
		}	
	}

}
