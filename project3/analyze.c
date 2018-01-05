/****************************************************/
/* File: analyze.c                                  */
/* Semantic analyzer implementation                 */
/* for the TINY compiler                            */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"
#include "symtab.h"
#include "analyze.h"
#include "util.h"

static int unchangeScope = FALSE; 

static ExpType retType = Void;
static int isArray = FALSE; 

ScopeList globalScope = NULL;

static int paramTypeStack[SIZE];
static int paramTypeStackTop = 0;
static int lastCallLineno = 0;

static int printCallLineno = FALSE;

/* Procedure traverse is a generic recursive 
 * syntax tree traversal routine:
 * it applies preProc in preorder and postProc 
 * in postorder to tree pointed to by t
 */
static void traverse( TreeNode * t,
               void (* preProc) (TreeNode *),
               void (* postProc) (TreeNode *) )
{ if (t != NULL)
  { preProc(t);
    { int i;
      for (i=0; i < MAXCHILDREN; i++)
        traverse(t->child[i],preProc,postProc);
    }
    postProc(t);
    traverse(t->sibling,preProc,postProc);
  }
}

static void endInsertNode( TreeNode * t)
{ if (t->nodekind == StmtK)
  { if (t->kind.stmt == CompK)
      del_cur_scope();
  }
}

static void symbError(TreeNode * t, char * name, char * message)
{ fprintf(listing,"error: %s \"%s\" at line %d\n",message,name,t->lineno);
  Error = TRUE;
}

/* Procedure insertNode inserts 
 * identifiers stored in t into 
 * the symbol table 
 */
static void insertNode( TreeNode * t)
{ switch (t->nodekind)
  { case StmtK:
      switch (t->kind.stmt)
	  {	case VarK:
		case ArrVarK:
		// variable declarations
		{ char * name;
		  if (t->is_array)
			name = t->attr.arr.name;
	      else
			name = t->attr.name;

		  if (st_lookup_excluding_parent(name) != NULL)
		  { symbError(t, name, "Undeclared variable");
		    break;
		  }

		  if (t->var_type->type == Void)
		  { symbError(t, name, "Variable type cannot be Void");
		    break;
		  }

		  st_insert(name, t->var_type->type, t->lineno, get_location(), t->nodekind, t->kind.stmt, t->kind.exp);
		}
		break;

		case ParamK:
		case ArrParamK:
		// function parameters
		{ char * name;
	      if (t->is_array)
			name = t->attr.arr.name;
		  else
			name = t->attr.name;

		  if (st_lookup(name) != NULL)
		  { symbError(t, name, "Already declared variable");
		    break;
		  }

		  if (t->var_type->type == Void)
		  { symbError(t, name, "Parameter type cannot be Void");
		    break;
		  }

		  st_insert(name, t->var_type->type, t->lineno, get_location(), t->nodekind, t->kind.stmt, t->kind.exp);
		}
		break;

		case FuncK:
		// function declarations
		{ char * name = t->attr.name;
		  if (st_lookup(name) != NULL)
		  { symbError(t, name, "Undeclared function");
		    break;
		  }

		  t->type = t->var_type->type;

		  st_insert(name, t->var_type->type, t->lineno, get_location(), t->nodekind, t->kind.stmt, t->kind.exp);
		  insert_scope(name);
		  unchangeScope = TRUE;
		}
		break;

		case CompK:
		// compound statement(add scope)
		{ char * name = get_cur_scope()->name;
		  if (unchangeScope)
			unchangeScope = FALSE;
		  else
		    insert_scope(name);
		}
		break;

		case CallK:
		// function call
		{ char * name = t->attr.name;
		  int lineno = t->lineno;
		  BucketList find_bucket = st_lookup(name);
		  ScopeList find_scope = NULL;
		  if (find_bucket == NULL)
		  {	symbError(t, name, "Undeclared call");
			break;
		  }

		  t->type = find_bucket->type;

		  find_scope = find_func_def_scope(name);
		  if (find_scope == NULL)
		  { symbError(t, name, "Undeclared call");
		    break;
		  }

		  t->param_size = find_scope->param_size;
		  t->param_list = find_scope->param_list;
		  
		  st_insert_lineno(find_bucket, lineno);
		}
		break;

        default:
          break;
      }
      break;
    case ExpK:
      switch (t->kind.exp)
      { case IdK:
		// load variable
		{ char * name = t->attr.name;
		  int lineno = t->lineno;
		  BucketList find_bucket = st_lookup(name);
		  if (find_bucket == NULL)
		  { symbError(t, name, "Undeclared variable");
		    break;
		  }

		  t->type = find_bucket->type;
		  if (t->child[0] == NULL &&
			  find_bucket->nodekind == StmtK &&
			   (find_bucket->kind.stmt == ArrParamK ||
			    find_bucket->kind.stmt == ArrVarK))
		    t->is_array = TRUE;

		  st_insert_lineno(find_bucket, lineno);
		}
		break;

        default:
          break;
      }
      break;

    default:
      break;
  }
}

void insert_input_func()
{ st_insert("input", Integer, 0, 0, StmtK, FuncK, UnknownK);
  insert_scope("input");
  del_cur_scope();
}

void insert_output_func()
{ st_insert("output", Void, 0, 0, StmtK, FuncK, UnknownK);
  insert_scope("output");
  st_insert("arg", Integer, 0, 0, StmtK, ParamK, UnknownK);
  del_cur_scope();
}

/* Function buildSymtab constructs the symbol 
 * table by preorder traversal of the syntax tree
 */
void buildSymtab(TreeNode * syntaxTree)
{ insert_scope("global");
  globalScope = get_cur_scope();
  insert_input_func();
  insert_output_func();

  traverse(syntaxTree,insertNode,endInsertNode);
  del_cur_scope();
  if (TraceAnalyze && !Error)
  { fprintf(listing,"\nSymbol table:\n\n");
    printSymTab(listing);
  }
}

static void typeError(TreeNode * t, char * message)
{ if (printCallLineno)
	fprintf(listing,"Type error at line %d: %s\n",lastCallLineno,message);
  else
	fprintf(listing,"Type error at line %d: %s\n",t->lineno,message);

  printCallLineno = FALSE;
  Error = TRUE;
}

static void pullParamTypes(int pull_num)
{ int i;
  for (i = 0; i < paramTypeStackTop; i++)
  { if (i < pull_num)
      continue;

	paramTypeStack[i - pull_num] = paramTypeStack[i];
  }
}

/* Procedure checkNode performs
 * type checking at a single tree node
 */
static void checkNode(TreeNode * t)
{ switch (t->nodekind) { 
  case StmtK:
    switch (t->kind.stmt) { 
	  case IfK:
	    if (t->child[0]->type == Void)
		  typeError(t, "invalid if condition");
	    break;

	  case WhileK:
	    if (t->child[0]->type == Void)
		  typeError(t, "invalid while condition");
		break;

	  case FuncK:
	    if (t->type != retType || t->is_array != isArray)
		  typeError(t, "return type inconsistance");

		retType = Void;
		isArray = FALSE;
	    break;

	  case ReturnK:
	    if (t->child[0] == NULL)
		  retType = Void;
		else if (t->child[0]->type == Integer)
		{ retType = Integer;

		  if (t->child[0]->is_array)
		    isArray = TRUE;
		}
		else
		  retType = Void;

		break;

	  case AssignK:
	    if (t->child[0]->type != t->child[1]->type || 
			t->child[0]->is_array != t->child[1]->is_array)
		  typeError(t, "assign type inconsistance");

		t->type = t->child[0]->type;
		t->is_array = t->child[0]->is_array;
	    break;

	  case CallK:
	  { int call_error = FALSE;
		int i;
		
		if (lastCallLineno != t->lineno && paramTypeStackTop - t->param_size != 0)
		{ lastCallLineno = t->lineno;
		  typeError(t, "invalid function call");
		  pullParamTypes(paramTypeStackTop - t->param_size);
		  paramTypeStackTop = t->param_size;
		  printCallLineno = TRUE;
		}
		else
		  lastCallLineno = t->lineno;

		for (i = t->param_size-1; i >= 0; i--)
		{ paramTypeStackTop--;
		  if (paramTypeStackTop < 0 || t->param_list[i] != paramTypeStack[paramTypeStackTop])
		  { call_error = TRUE;
			break;
		  }
		}

		if (call_error)
		  typeError(t, "invalid function call");

		if (paramTypeStackTop < 0)
		  paramTypeStackTop = 0;

		if (t->is_argu)
		  paramTypeStack[paramTypeStackTop++] = PARAMINTEGER;
	  }
	  break;
	}
	break;

	case ExpK:
	switch (t->kind.exp) {
	  // I will set op result to integer (1: true | 0: false)
	  case OpK:
	    if (t->child[0]->type != t->child[1]->type ||
			t->child[0]->is_array || t->child[1]->is_array)
		  typeError(t, "op type inconsistance");

		t->type = Integer;
		t->is_array = FALSE;

		if (t->is_argu)
		  paramTypeStack[paramTypeStackTop++] = PARAMINTEGER;
		break;

	  case IdK:
	    if (t->is_argu)
		{ if (t->is_array)
		    paramTypeStack[paramTypeStackTop++] = PARAMINTEGERARRAY;
		  else
			paramTypeStack[paramTypeStackTop++] = PARAMINTEGER;
		}
		break;

	  case ConstK:
	    t->type = Integer;
		if (t->is_argu)
	      paramTypeStack[paramTypeStackTop++] = PARAMINTEGER;
		break;
	}
	break;
  }
}

void nullProc() {}

/* Procedure typeCheck performs type checking 
 * by a postorder syntax tree traversal
 */
void typeCheck(TreeNode * syntaxTree)
{ traverse(syntaxTree,nullProc,checkNode);
}
