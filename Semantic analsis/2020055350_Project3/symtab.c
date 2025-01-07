/****************************************************/
/* File: symtab.c                                   */
/* Symbol table implementation for the TINY compiler*/
/* (allows only one symbol table)                   */
/* Symbol table is implemented as a chained         */
/* hash table                                       */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "symtab.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static char* NodeTypeToString(NodeType type) {
    if (type == Integer) return "int";
    else if (type == Undetermined) return "undetermined";
    else if (type == IntegerArray) return "int[]";
    else if (type == Void) return "void";
    else if (type == VoidArray) return "void[]";
    else return "unknown";
}
static char* SymbolKindToString(SymbolKind kind)
{
	 if(kind == FunctionSym) return "Function";
	 else if(kind == VariableSym) return "Variable";
	 else return "Unknown";
}

#define HASH_SHIFT 4
static int hash(char *key)
{
	int temp = 0;
	int i = 0;
	while (key[i] != '\0')
	{
		temp = ((temp << HASH_SHIFT) + key[i]) % HASH_TABLE_SIZE;
		++i;
	}
	return temp;
}


static ScopeEntryList allScopes = NULL;

ScopeEntryRec *InsertScope(char *name, ScopeEntryRec *parentScope, TreeNode *functionNode)
{
	char *scopeName = NULL;
	if (name == NULL)
	{
    size_t length = strlen(parentScope->name);
    scopeName = (char *)malloc(length + 6); 
    if (scopeName != NULL) {
        strcpy(scopeName, parentScope->name);        
        strcat(scopeName, ".");                
        char numStr[5];                       
        sprintf(numStr, "%d", parentScope->nestedScopeCount++);
        strcat(scopeName, numStr);             
    }
	}
	else
	{
    	size_t length = strlen(name);
    	scopeName = (char *)malloc(length + 1);
   	 if (scopeName != NULL) {
        strcpy(scopeName, name);              
    	}
	}

	int redefined = (parentScope != NULL && parentScope->status == defined) ? TRUE : FALSE;
	ScopeEntryRec *tmpScope = allScopes;
	while (tmpScope != NULL)
	{
		if (strcmp(scopeName, tmpScope->name) == 0)
		{
			redefined = TRUE;
		}
		if (tmpScope->next == NULL) break;
		tmpScope = tmpScope->next;
	}

	ScopeEntryRec *scope = (ScopeEntryRec *)malloc(sizeof(ScopeEntryRec));
	scope->name = scopeName;
	scope->status = redefined == TRUE ? defined : nonerror;
	scope->functionNode = functionNode;
	for (int i = 0; i < HASH_TABLE_SIZE; ++i) scope->symbols[i] = NULL;
	scope->symbolCount = 0;
	scope->nestedScopeCount = 0;
	scope->parentScope = parentScope;
	if (tmpScope == NULL) allScopes = scope;
	else
		tmpScope->next = scope;
	scope->next = NULL;
	return scope;
}


SymbolEntryRec *InsertSymbol(ScopeEntryRec *activeScope, char *name, NodeType type, SymbolKind kind, int lineno, TreeNode *node)
{
	int hashIdx = hash(name);
	SymbolEntryRec *tmpSymbol = activeScope->symbols[hashIdx];
	ErrorState status = nonerror;
	while (tmpSymbol != NULL)
	{
		if( strcmp(name, tmpSymbol->name) == 0 )
		{
			if (tmpSymbol->status == defined) status = defined;
			else if( tmpSymbol->status == undeclared)
			{
				tmpSymbol->type = type;
				tmpSymbol->status = node == NULL ? undeclared : nonerror;
				return tmpSymbol;
			}
		}
		if (tmpSymbol->next == NULL) break;
		tmpSymbol = tmpSymbol->next;
	}

	SymbolEntryRec *symbol = (SymbolEntryRec *)malloc(sizeof(SymbolEntryRec));
	symbol->name = name;
	symbol->status = status;
	symbol->type = type;
	symbol->kind = kind;
	symbol->lineUsage = (LineUsage)malloc(sizeof(LineUsageRec));
	symbol->lineUsage->lineno = lineno;
	symbol->lineUsage->next = NULL;
	symbol->memoryLocation = activeScope->symbolCount++;
	if (tmpSymbol == NULL) activeScope->symbols[hashIdx] = symbol;
	else
		tmpSymbol->next = symbol;
	symbol->next = NULL;
	symbol->node = node;
	if( node == NULL ) symbol->status = undeclared;

	return symbol;
}

SymbolEntryRec *InsertSymbolIntoScope(ScopeEntryRec *activeScope, char *name, int lineno)
{
	int hashIdx = hash(name);
	ScopeEntryRec *scope = activeScope;
	SymbolEntryRec *symbol = NULL;
	while (scope != NULL)
	{
		symbol = scope->symbols[hashIdx];
		while ((symbol != NULL) && (strcmp(name, symbol->name) != 0)) symbol = symbol->next;
		
		if (symbol == NULL) scope = scope->parentScope;
		else
			break;
	}

	LineUsageRec *line = symbol->lineUsage;
	while (line->next != NULL) line = line->next;
	line->next = (LineUsageRec *)malloc(sizeof(LineUsageRec));
	line->next->lineno = lineno;
	line->next->next = NULL;

	return symbol;
}

SymbolEntryRec *SearchSymbol(ScopeEntryRec *activeScope, char *name)
{
	int hashIdx = hash(name);
	ScopeEntryRec *scope = activeScope;
	SymbolEntryRec *symbol = NULL;

	while (scope != NULL)
	{
		symbol = scope->symbols[hashIdx];
		while ((symbol != NULL) && (strcmp(name, symbol->name) != 0)) symbol = symbol->next;
		
		if (symbol == NULL) scope = scope->parentScope;
		else
			return symbol;
	}

	return NULL;
}

SymbolEntryRec *SearchSymbolInScope(ScopeEntryRec *activeScope, char *name)
{
	int hashIdx = hash(name);
	ScopeEntryRec *scope = activeScope;
	SymbolEntryRec *symbol = scope->symbols[hashIdx];
	while ((symbol != NULL) && (strcmp(name, symbol->name) != 0)) symbol = symbol->next;

	return symbol;
}

SymbolEntryRec *SearchSymbolByKind(ScopeEntryRec *activeScope, char *name, SymbolKind kind)
{
	int hashIdx = hash(name);
	ScopeEntryRec *scope = activeScope;
	SymbolEntryRec *symbol = NULL;

	while (scope != NULL)
	{
		symbol = scope->symbols[hashIdx];
		while ((symbol != NULL) && ((strcmp(name, symbol->name) != 0) || (symbol->kind != kind))) symbol = symbol->next;

		if (symbol == NULL) scope = scope->parentScope;
		else
			return symbol;
	}

	return NULL;
}

void DisplaySymbolTable(FILE *listing, ScopeEntryRec *rootScope)
{
	fprintf(listing, "\n\n< Symbol Table >\n");
	fprintf(listing, " Symbol Name   Symbol Kind   Symbol Type    Scope Name   Location  Line Numbers\n");
	fprintf(listing, "-------------  -----------  -------------  ------------  --------  ------------\n");
	ScopeEntryRec *scope = allScopes;
	while (scope != NULL)
	{
		for (int i = 0; i < HASH_TABLE_SIZE; ++i)
		{
			SymbolEntryRec *symbol = scope->symbols[i];
			while (symbol != NULL)
			{
				fprintf(listing,
						"%-13s  %-11s  %-13s  %-12s  %-8d ",
						symbol->name,
						SymbolKindToString(symbol->kind),
						NodeTypeToString(symbol->type),
						scope->name,
						symbol->memoryLocation);
				LineUsageRec *line = symbol->lineUsage;
				while (line != NULL)
				{
					fprintf(listing, "%4d ", line->lineno);
					line = line->next;
				}
				fprintf(listing, "\n");
				symbol = symbol->next;
			}
		}
		scope = scope->next;
	}
	
	fprintf(listing, "\n\n< Functions >\n");
	fprintf(listing, "Function Name   Return Type   Parameter Name  Parameter Type\n");
	fprintf(listing, "-------------  -------------  --------------  --------------\n");
	scope = allScopes;
	while (scope != NULL)
	{
		for (int i = 0; i < HASH_TABLE_SIZE; ++i)
		{
			SymbolEntryRec *symbol = scope->symbols[i];
			while (symbol != NULL)
			{
				if (symbol->kind == FunctionSym)
				{
					fprintf(listing, "%-13s  %-13s ", symbol->name, NodeTypeToString(symbol->type));
					if (symbol->type == Undetermined) fprintf(listing, " %-14s  %-12s\n", "", NodeTypeToString(Undetermined));
					else
					{
						TreeNode *param = symbol->node->child[0];
						if (param->type == Void) fprintf(listing, " %-14s  %-12s\n", "", NodeTypeToString(Void));
						else
						{
							fprintf(listing, "\n");
							while (param != NULL)
							{
								fprintf(listing, "%-13s  %-13s  %-14s  %-12s\n", "-", "-", param->name, NodeTypeToString(param->type));
								param = param->sibling;
							}
						}
					}
				}
				symbol = symbol->next;
			}
		}
		scope = scope->next;
	}
	fprintf(listing, "\n\n< Global Symbols >\n");
	fprintf(listing, " Symbol Name   Symbol Kind   Symbol Type\n");
	fprintf(listing, "-------------  -----------  -------------\n");
	for (int i = 0; i < HASH_TABLE_SIZE; ++i)
	{
		SymbolEntryRec *symbol = rootScope->symbols[i];
		while (symbol != NULL)
		{
			fprintf(listing, "%-13s  %-11s  %-13s\n", symbol->name, SymbolKindToString(symbol->kind), NodeTypeToString(symbol->type));
			symbol = symbol->next;
		}
	}
		fprintf(listing, "\n\n< Scopes >\n");
		fprintf(listing, " Scope Name   Nested Level   Symbol Name   Symbol Type\n");
	fprintf(listing, "------------  ------------  -------------  -----------\n");
	scope = allScopes;
	while (scope != NULL)
	{
		if (scope == rootScope)
		{
			scope = scope->next;
			continue;
		}

		int PrintSymbol = FALSE;
		for (int i = 0; i < HASH_TABLE_SIZE; ++i)
		{
			SymbolEntryRec *symbol = scope->symbols[i];
			while (symbol != NULL)
			{
				int nested_level = 0;
				ScopeEntryRec *activeScope = scope;
				while (scope != rootScope)
				{
					scope = scope->parentScope;
					nested_level++;
				}
				scope = activeScope;
				fprintf(listing, "%-12s  %-12d  %-13s  %-11s\n", scope->name, nested_level, symbol->name, NodeTypeToString(symbol->type));
				PrintSymbol = TRUE;
				symbol = symbol->next;
			}
		}
		if (PrintSymbol) fprintf(listing, "\n");
		scope = scope->next;
	}

}



