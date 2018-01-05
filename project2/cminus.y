/****************************************************/
/* File: tiny.y                                     */
/* The TINY Yacc/Bison specification file           */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/
%{
#define YYPARSER /* distinguishes Yacc output from other code files */

#include "globals.h"
#include "util.h"
#include "scan.h"
#include "parse.h"

#define YYSTYPE TreeNode *
static char * savedName; /* for use in assignments */
static int savedLineNo;  /* ditto */
static int savedNum; /* trans char * to int */
static TreeNode * savedTree; /* stores syntax tree for later return */
static int yylex(void); // added 11/2/11 to ensure no conflict with lex
int yyerror(char * message);

%}

%token IF ELSE WHILE RETURN INT VOID/* discarded */ THEN END REPEAT UNTIL READ WRITE
%token ID NUM 
%token ASSIGN EQ NE LT LE GT GE PLUS MINUS TIMES OVER LPAREN RPAREN LBRACE RBRACE LCURLY RCURLY SEMI COMMA
%token ERROR 

%% /* Grammar for Cminus */
program		: dec_list {
				savedTree = $1; 
			}
			;
dec_list	: dec_list dec { 
				YYSTYPE t = $1;
					
				if (t != NULL) {
					while (t->sibling != NULL)
						t = t->sibling;

					t->sibling = $2;
					$$ = $1;
				}
				else
					$$ = $2;
			}
			| dec { $$ = $1; }
			;
dec			: var_dec { $$ = $1; }
			| func_dec { $$ = $1; }
			;
var_dec		: type_spec save_name SEMI {
				$$ = newStmtNode(VarK);
				$$->attr.name = savedName;
				$$->var_type = $1;
              }
			| type_spec save_name LBRACE save_num RBRACE SEMI {
				$$ = newStmtNode(ArrVarK);
				$$->attr.arr.name = savedName;
				$$->attr.arr.size = savedNum;
				$$->var_type = $1;
			  }
			;
save_name	: ID { savedName = copyString(tokenString); }

save_num	: NUM { savedNum = atoi(tokenString); }

type_spec	: INT { 
				$$ = newStmtNode(TypeK); 
				$$->type = Integer;
			}
			| VOID { 
				$$ = newStmtNode(TypeK);
				$$->type = Void; 
			}
			;
func_dec	: type_spec save_name {
				$$ = newStmtNode(FuncK);
				$$->attr.name = savedName;
			  }
			  LPAREN params RPAREN comp_stmt {
				$$ = $3;
				$$->var_type = $1;
				$$->child[0] = $5;
				$$->child[1] = $7;
			}
			;
params		: param_list { $$ = $1; }
			| VOID { $$ = newExpNode(NullParamK); }
			;
param_list	: param_list COMMA param {
				YYSTYPE t = $1;
				if (t != NULL) {
					while (t->sibling != NULL)
						t = t->sibling;

					t->sibling = $3;
					$$ = $1;
				}
				else
					$$ = $3;
			}
			| param { $$ = $1; }
			;
param		: type_spec save_name {
				$$ = newStmtNode(ParamK);
				$$->attr.name = savedName;
				$$->var_type = $1;
			}
			| type_spec save_name {
				$$ = newStmtNode(ArrParamK);
				$$->var_type = $1;
				$$->attr.arr.name = savedName;
			  }
			  LBRACE RBRACE {
				$$ = $3;
			  }
			;
comp_stmt	: LCURLY local_decs stmt_list RCURLY {
				$$ = newStmtNode(CompK);
				$$->child[0] = $2;
				$$->child[1] = $3;
			}
			;
local_decs	: local_decs var_dec {
				YYSTYPE t = $1;
				if (t != NULL) {
					while (t->sibling != NULL)
						t = t->sibling;

					t->sibling = $2;
					$$ = $1;
				}
				else
					$$ = $2;
			}
			| /* empty */ { $$ = NULL; }
			;
stmt_list	: stmt_list stmt {
				YYSTYPE t = $1;
				if (t != NULL) {
					while (t->sibling != NULL)
						t = t->sibling;

					t->sibling = $2;
					$$ = $1;
				}
				else
					$$ = $2;
			}
			| /* empty */ { $$ = NULL; }
			;
stmt		: exp_stmt { $$ = $1; }
			| comp_stmt { $$ = $1; }
			| sel_stmt { $$ = $1; }
			| while_stmt { $$ = $1; }
			| ret_stmt { $$ = $1; }
			;
exp_stmt	: exp SEMI { $$ = $1; }
			| SEMI { $$ = NULL; }
			;
sel_stmt	: IF LPAREN exp RPAREN stmt {
				$$ = newStmtNode(IfK);
				$$->child[0] = $3;
				$$->child[1] = $5;
			}
			| IF LPAREN exp RPAREN stmt ELSE stmt {
				$$ = newStmtNode(IfK);
				$$->child[0] = $3;
				$$->child[1] = $5;
				$$->child[2] = $7;
			}
			;
while_stmt	: WHILE LPAREN exp RPAREN stmt {
				$$ = newStmtNode(WhileK);
				$$->child[0] = $3;
				$$->child[1] = $5;
			}
			;
ret_stmt	: RETURN SEMI {	$$ = newStmtNode(ReturnK); }
			| RETURN exp SEMI {
				$$ = newStmtNode(ReturnK);
				$$->child[0] = $2;
			}
			;
exp			: var ASSIGN exp {
				$$ = newStmtNode(AssignK);
				$$->child[0] = $1;
				$$->child[1] = $3;
			}
			| simple_exp { $$ = $1; }
			;
var			: save_name {
				$$ = newExpNode(IdK);
				$$->attr.name = savedName;
			}
			| save_name {
				$$ = newExpNode(ArrIdK);
				$$->attr.name = savedName;
			}
			LBRACE exp RBRACE {
                $$ = $2;
				$$->child[0] = $4;
			}
			;
simple_exp	: additive_exp LT additive_exp {
				$$ = newExpNode(OpK);
				$$->child[0] = $1;
				$$->child[1] = $3;
				$$->attr.op = LT;
			  }
			| additive_exp LE additive_exp {
				$$ = newExpNode(OpK);
				$$->child[0] = $1;
				$$->child[1] = $3;
				$$->attr.op = LE;
			  }
			| additive_exp GT additive_exp {
				$$ = newExpNode(OpK);
				$$->child[0] = $1;
				$$->child[1] = $3;
				$$->attr.op = GT;
			  }
			| additive_exp GE additive_exp {
				$$ = newExpNode(OpK);
				$$->child[0] = $1;
				$$->child[1] = $3;
				$$->attr.op = GE;
			  }
			| additive_exp EQ additive_exp {
				$$ = newExpNode(OpK);
				$$->child[0] = $1;
				$$->child[1] = $3;
				$$->attr.op = EQ;
			  }
			| additive_exp NE additive_exp {
				$$ = newExpNode(OpK);
				$$->child[0] = $1;
				$$->child[1] = $3;
				$$->attr.op = NE;
			  }
			| additive_exp { $$ = $1; }
			;
additive_exp: additive_exp PLUS term {
				$$ = newExpNode(OpK);
				$$->child[0] = $1;
				$$->child[1] = $3;
				$$->attr.op = PLUS;
			}
			| additive_exp MINUS term {
				$$ = newExpNode(OpK);
				$$->child[0] = $1;
				$$->child[1] = $3;
				$$->attr.op = MINUS;
			}
			| term { $$ = $1; }
			;
term		: term TIMES factor {
				$$ = newExpNode(OpK);
				$$->child[0] = $1;
				$$->child[1] = $3;
				$$->attr.op = TIMES;
			}
			| term OVER factor {
				$$ = newExpNode(OpK);
				$$->child[0] = $1;
				$$->child[1] = $3;
				$$->attr.op = OVER;
			}
			| factor { $$ = $1; }
			;
factor		: LPAREN exp RPAREN { $$ = $2; }
			| var { $$ = $1; }
			| call { $$ = $1; }
			| save_num {
				$$ = newExpNode(ConstK);
				$$->attr.val = savedNum;
			}
			;
call		: save_name {
				$$ = newStmtNode(CallK);
				$$->attr.name = savedName;
			  }
			  LPAREN args RPAREN {
				$$ = $2;
				$$->child[0] = $4;
			}
			;
args		: arg_list { $$ = $1; }
			| /* empty */ { $$ = NULL; }
			;
arg_list	: arg_list COMMA exp {
				YYSTYPE t = $1;
				if (t != NULL) {
					while (t->sibling != NULL)
						t = t->sibling;

					t->sibling = $3;
					$$ = $1;
				}
				else
					$$ = $3;
			}
			| exp { $$ = $1; }
			;
%%

int yyerror(char * message)
{ fprintf(listing,"Syntax error at line %d: %s\n",lineno,message);
  fprintf(listing,"Current token: ");
  printToken(yychar,tokenString);
  Error = TRUE;
  return 0;
}

/* yylex calls getToken to make Yacc/Bison output
 * compatible with ealier versions of the TINY scanner
 */
static int yylex(void)
{ return getToken(); }

TreeNode * parse(void)
{ yyparse();
  return savedTree;
}

