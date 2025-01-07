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

#define HASH_TABLE_SIZE 211



typedef enum ErrorState
{
	nonerror,
	defined,
	undeclared
} ErrorState;

typedef struct LineUsageRec
{
	int lineno;
	struct LineUsageRec *next;
} LineUsageRec, *LineUsage;

typedef struct SymbolEntryRec
{
	char *name;
	ErrorState status;
	NodeType type;
	SymbolKind kind;
	LineUsage lineUsage;
	int memoryLocation;
	TreeNode *node;
	struct SymbolEntryRec *next;
} SymbolEntryRec, *SymbolEntryList;

typedef struct ScopeEntryRec
{
	char *name;
	ErrorState status;
	TreeNode *functionNode;
	SymbolEntryList symbols[HASH_TABLE_SIZE];
	int symbolCount;
	int nestedScopeCount;
	struct ScopeEntryRec *parentScope;
	struct ScopeEntryRec *next;
} ScopeEntryRec, *ScopeEntryList;



ScopeEntryRec* InsertScope(char *name, ScopeEntryRec *parentScope, TreeNode *functionNode);
SymbolEntryRec* InsertSymbol(ScopeEntryRec *activeScope, char *name, NodeType type, SymbolKind kind, int lineno, TreeNode *node);
SymbolEntryRec* InsertSymbolIntoScope(ScopeEntryRec *activeScope, char *name, int lineno);
SymbolEntryRec* SearchSymbol(ScopeEntryRec *activeScope, char *name);
SymbolEntryRec* SearchSymbolInScope(ScopeEntryRec *activeScope, char *name);
SymbolEntryRec* SearchSymbolByKind(ScopeEntryRec *activeScope, char *name, SymbolKind kind);

void DisplaySymbolTable(FILE *listing, ScopeEntryRec *rootScope);

#endif
