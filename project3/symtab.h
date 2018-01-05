/****************************************************/
/* File: symtab.h                                   */
/* Symbol table interface for the TINY compiler     */
/* (allows only one symbol table)                   */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#ifndef _SYMTAB_H_
#define _SYMTAB_H_

#include "globals.h"

/* SIZE is the size of the hash table */
#define SIZE 211

/* the list of line numbers of the source 
 * code in which a variable is referenced
 */
typedef struct LineListRec
   { int lineno;
     struct LineListRec * next;
   } * LineList;

/* The record in the bucket lists for
 * each variable, including name, 
 * assigned memory location, and
 * the list of line numbers in which
 * it appears in the source code
 */
typedef struct BucketListRec
   { char * name;
     ExpType type;
     LineList lines;
     int memloc ; /* memory location for variable */
	 NodeKind nodekind;
	 union { StmtKind stmt; ExpKind exp; } kind;
     struct BucketListRec * next;
   } * BucketList;

typedef struct ScopeListRec
   { char * name;
     BucketList bucket[SIZE];
	 struct ScopeListRec * parent;
	 int nested_level;
	 int param_size;
	 int * param_list;
   } * ScopeList;

int get_location();

void insert_scope(char * name);
ScopeList get_cur_scope();
void del_cur_scope();
void push_scope();
ScopeList find_func_def_scope(char * name);

/* Procedure st_insert inserts line numbers and
 * memory locations into the symbol table
 * loc = memory location is inserted only the
 * first time, otherwise ignored
 */
void st_insert( char * name, ExpType type, int lineno, int loc, NodeKind nodekind, StmtKind stmt, ExpKind exp );
void st_insert_lineno(BucketList bucket, int lineno);

/* Function st_lookup returns the memory 
 * location of a variable or -1 if not found
 */
BucketList st_lookup ( char * name );
BucketList st_lookup_excluding_parent ( char * name );

/* Procedure printSymTab prints a formatted 
 * listing of the symbol table contents 
 * to the listing file
 */
void printSymTab(FILE * listing);
void printGlobalInfo(FILE * listing);
void printLocalsInfo(FILE * listing);

#endif
