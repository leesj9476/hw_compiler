/****************************************************/
/* File: symtab.c                                   */
/* Symbol table implementation for the TINY compiler*/
/* (allows only one symbol table)                   */
/* Symbol table is implemented as a chained         */
/* hash table                                       */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"
#include "globals.h"

/* SHIFT is the power of two used as multiplier
   in hash function  */
#define SHIFT 4

/* the hash function */
static int hash ( char * key )
{ int temp = 0;
  int i = 0;
  while (key[i] != '\0')
  { temp = ((temp << SHIFT) + key[i]) % SIZE;
    ++i;
  }
  return temp;
}

/* the hash table */
static ScopeList scopeTable[SIZE];
static ScopeList scopeStack[SIZE];
static int varLocation[SIZE];
static int nScope = 0;
static int nScopeTop = 0;
static int nestedLv= 0;

extern ScopeList globalScope;

int get_location()
{ return varLocation[nScopeTop - 1]++;
}

ScopeList get_cur_scope()
{ return scopeStack[nScopeTop - 1];
}

void insert_scope( char * name )
{ ScopeList parent = get_cur_scope();
  scopeStack[nScopeTop] = (ScopeList) malloc(sizeof(struct ScopeListRec));
  scopeStack[nScopeTop]->name = name;
  scopeStack[nScopeTop]->parent = parent;
  scopeStack[nScopeTop]->param_size = 0;

  scopeStack[nScopeTop]->nested_level = nestedLv;
  if (nestedLv == 1)
    scopeStack[nScopeTop]->param_list = (int *) malloc(sizeof(int) * SIZE);

  scopeTable[nScope] = scopeStack[nScopeTop];

  nestedLv++;
  nScopeTop++;
  nScope++;
}

void push_scope( ScopeList scope )
{ scopeStack[nScopeTop] = scope;
  varLocation[nScopeTop] = 0;
  nScopeTop++;
}

void del_cur_scope()
{ nScopeTop--;
  nestedLv--;
}

ScopeList find_func_def_scope(char * name)
{ int i;
  for (i=0;i<nScope;i++)
  { if (scopeTable[i]->nested_level == 1 && strcmp(scopeTable[i]->name, name) == 0)
      return scopeTable[i];
  }
}

/* Procedure st_insert inserts line numbers and
 * memory locations into the symbol table
 * loc = memory location is inserted only the
 * first time, otherwise ignored
 */
void st_insert( char * name, ExpType type, int lineno, int loc, NodeKind nodekind, StmtKind stmt, ExpKind exp )
{ ScopeList sl = get_cur_scope();
  int bh;
  BucketList l = NULL;

  bh = hash(name);
  l = sl->bucket[bh];

  while ((l != NULL) && (strcmp(name,l->name) != 0))
    l = l->next;
  if (l == NULL) /* variable not yet in table */
  { l = (BucketList) malloc(sizeof(struct BucketListRec));
    l->name = name;
	l->type = type;
    l->lines = (LineList) malloc(sizeof(struct LineListRec));
    l->lines->lineno = lineno;
    l->memloc = loc;

    if (nodekind == StmtK)
	{ l->kind.stmt = stmt;
	  if (stmt == ParamK)
	  { sl->param_list[sl->param_size] = PARAMINTEGER;
	    sl->param_size++;
	  }
	  else if (stmt == ArrParamK)
	  { sl->param_list[sl->param_size] = PARAMINTEGERARRAY;
	    sl->param_size++;
	  }
	}
	else
	  l->kind.exp = exp;

    l->lines->next = NULL;
    l->next = sl->bucket[bh];
    sl->bucket[bh] = l;
  }
} /* st_insert */

void st_insert_lineno( BucketList bucket, int lineno )
{ LineList t = bucket->lines;
  while (t->next != NULL)
  { if (t->lineno == lineno)
      return ;

    t = t->next;
  }

  if (t->lineno == lineno)
	return ;

  t->next = (LineList) malloc(sizeof(struct LineListRec));
  t = t->next;

  t->lineno = lineno;
  t->next = NULL;
}

BucketList st_lookup ( char * name )
{ ScopeList cur_scope = get_cur_scope();

  while (cur_scope != NULL)
  { int i;
	for (i=0;i<SIZE;i++)
    { BucketList cur_bucket = cur_scope->bucket[i];
	  while (cur_bucket != NULL)
      { if (strcmp(cur_bucket->name, name) == 0)
	      return cur_bucket;

		cur_bucket = cur_bucket->next;
	  }
	}

    cur_scope = cur_scope->parent;
  }

  return NULL;
}

BucketList st_lookup_excluding_parent ( char * name )
{ ScopeList cur_scope = get_cur_scope();

  int i;
  for (i=0;i<SIZE;i++)
  { BucketList cur_bucket = cur_scope->bucket[i];
    while (cur_bucket != NULL)
	{ if (strcmp(cur_bucket->name, name) == 0)
	    return cur_bucket;

	  cur_bucket = cur_bucket->next;
	}
  }

  return NULL;
}

/* Procedure printSymTab prints a formatted 
 * listing of the symbol table contents 
 * to the listing file
 */
void printSymTab(FILE * listing)
{ int i;
  printGlobalInfo(listing);
  printLocalsInfo(listing);
} /* printSymTab */

void printGlobalInfo(FILE * listing)
{ int i, j, k;
  BucketList cur_global_info = NULL;
  fprintf(listing,"<FUNCTIONS AND GLOBAL VARIABLES>\n");
  fprintf(listing,"-------------  ---------  -----------\n");
  fprintf(listing,"   ID Name      ID Type    Data Type\n");
  fprintf(listing,"-------------  ---------  -----------\n");
  for (i=0;i<SIZE;i++)
  { BucketList cur = globalScope->bucket[i];
	while (cur != NULL)
    { fprintf(listing,"%-14s  ",cur->name);
	  
      if (cur->nodekind == StmtK)
	  { switch (cur->kind.stmt) {
	    case FuncK:
		  switch (cur->type) {
		  case Void:
		    fprintf(listing,"Function   Void\n");
			break;
		  case Integer:
		    fprintf(listing,"Function   Integer\n");
			break;
		  }
		  break;

	    case VarK:
		  switch (cur->type) {
		  case Void:
	        fprintf(listing,"Variable   Void\n");
			break;
		  case Integer:
	        fprintf(listing,"Variable   Integer\n");
		    break;
		  }
		  break;
		    
	    case ArrVarK:
		  switch (cur->type) {
		  case Void:
	        fprintf(listing,"Variable   Void\n");
			break;
		  case Integer:
	        fprintf(listing,"Variable   IntegerArray\n");
		    break;
		  }
		  break;
		}
	  }

	  cur = cur->next;
	}
  }

  fprintf(listing,"\n<FUNCTION DECLARATIONS>\n");
  for (i=0;i<SIZE;i++)
  { ScopeList cur_scope = NULL;
    cur_global_info = globalScope->bucket[i];
	while (cur_global_info != NULL)
	{ int param = 0;
	  BucketList cur = NULL;

	  // check if current bucket is function
	  if (cur_global_info->nodekind == StmtK && cur_global_info->kind.stmt == FuncK)
	  { char * name = cur_global_info->name;
	    cur_scope = find_func_def_scope(name);
	  }

	  // cannot find functions scope
      if (cur_scope == NULL)
	  { cur_global_info = cur_global_info->next;
		continue;
	  }
	  
	  fprintf(listing,"-------------  -------------\n");
	  fprintf(listing,"Function Name    Data Type\n");
	  fprintf(listing,"-------------  -------------\n");
	  fprintf(listing,"%-14s  ",cur_scope->name);
	  switch (cur_global_info->type) {
	  case Void:
		fprintf(listing,"Void\n\n");
		break;

	  case Integer:
		fprintf(listing,"Integer\n\n");
		break;
	  }

	  // print function parameters
	  fprintf(listing,"-------------------  -------------\n");
	  fprintf(listing,"Function Parameters    Data Type\n");
	  fprintf(listing,"-------------------  -------------\n");

	  // find parameters info from found scope
	  for (j=0;j<SIZE;j++)
	  { BucketList cur_bucket = cur_scope->bucket[j];
		while (cur_bucket != NULL) 
	    { if (cur_bucket->nodekind == StmtK)
		  { switch (cur_bucket->kind.stmt) {
		    case ParamK:
			  param++;
			  switch (cur_bucket->type) {
			  case Void:
		        fprintf(listing,"%-19s  Void\n",cur_bucket->name);
				break;
			  case Integer:
                fprintf(listing,"%-19s  Integer\n",cur_bucket->name);
				break;
			  }		  
			  break;

		    case ArrParamK:
			  param++;
			  switch (cur_bucket->type) {
			  case Void:
		        fprintf(listing,"%-19s  Void\n",cur_bucket->name);
			    break;
			  case Integer:
                fprintf(listing,"%-19s  IntegerArray\n",cur_bucket->name);
				break;
			  }
			  break;
		    }
		  }

		  cur_bucket = cur_bucket->next;
		}
	  }

	  if (param==0)
	    fprintf(listing,"void                 Void\n\n");
	  else
		fprintf(listing,"\n");

	  cur_global_info = cur_global_info->next;
	}
  }
}

void printLocalsInfo(FILE * listing)
{ int i, j;
  fprintf(listing,"\n<FUNCTIONS PARAMETERS AND LOCAL VARIABLES>\n");
  for (i=3;i<nScope;i++)
  { fprintf(listing,"function name: %s (nested level: %d)\n",scopeTable[i]->name, scopeTable[i]->nested_level);
	fprintf(listing,"-------------  ---------  -------------  ---------\n");
    fprintf(listing,"   ID Name      ID Type     Data Type     Line No\n");
	fprintf(listing,"-------------  ---------  -------------  ---------\n");

    for (j=0;j<SIZE;j++)
	{ BucketList cur = scopeTable[i]->bucket[j];
	  while (cur != NULL)
	  { LineList line = cur->lines;
		fprintf(listing,"%-13s  Variable   ",cur->name);
	    if (cur->nodekind == StmtK) {
		  switch (cur->kind.stmt) {
		  case ParamK:
		  case VarK:
		    if (cur->type == Void)
			  fprintf(listing,"Void           ");
			else
		      fprintf(listing,"Integer        ");

		    break;
		  
		  case ArrParamK:
		  case ArrVarK:
		    if (cur->type == Void)
			  fprintf(listing,"Void           ");
			else
		      fprintf(listing,"IntegerArray   ");

		    break;

		  default:
		    cur = cur->next;
			continue;
		  }
		}

        while (line != NULL)
		{ fprintf(listing,"%d ", line->lineno);
		  line = line->next;
		}
		fprintf(listing,"\n");

		cur = cur->next;
	  }
	}

    fprintf(listing,"\n");
  }
}	
