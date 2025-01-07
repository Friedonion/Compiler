/****************************************************/
/* File: analyze.c                                  */
/* Semantic analyzer implementation                 */
/* for the TINY compiler                            */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "analyze.h"
#include "globals.h"
#include "symtab.h"
#include "util.h"

static ScopeEntryRec *rootScope = NULL;
static ScopeEntryRec *activeScope = NULL;

/*
fprintf(listing, "Error: undeclared function \"%s\" is called at line %d\n", name, lineno);
fprintf(listing, "Error: undeclared variable \"%s\" is used at line %d\n", name, lineno);
fprintf(listing, "Error: The void-type variable is declared at line %d (name : \"%s\")\n", lineno, name);
fprintf(listing, "Error: Invalid array indexing at line %d (name : \"%s\"). indices should be integer\n", lineno, name);
fprintf(listing, "Error: Invalid array indexing at line %d (name : \"%s\"). indexing can only allowed for int[] variables\n", lineno, name);
fprintf(listing, "Error: Invalid function call at line %d (name : \"%s\")\n", lineno, name);
fprintf(listing, "Error: Invalid return at line %d\n", lineno);
fprintf(listing, "Error: invalid assignment at line %d\n", lineno);
fprintf(listing, "Error: invalid operation at line %d\n", lineno);
fprintf(listing, "Error: invalid condition at line %d\n", lineno); 
*/


static void handleRedefinitionError(char *name, int lineno, SymbolEntryList symbol) //check
{
	Error = TRUE;
	fprintf(listing, "Error: Symbol \"%s\" is redefined at line %d (already defined at line", name, lineno);
	while (symbol != NULL)
	{
		if (strcmp(name, symbol->name) == 0)
		{
			symbol->status = defined;
			if (symbol->node->scope != NULL) symbol->node->scope->status = defined;
			fprintf(listing, " %d", symbol->lineUsage->lineno);
		}
		symbol = symbol->next;
	}
	fprintf(listing, ")\n");
}

static SymbolEntryRec *UndeclaredFunctionError(ScopeEntryRec *activeScope, TreeNode *node)  // check
{
	Error = TRUE;
	fprintf(listing, "Error: undeclared function \"%s\" is called at line %d\n", node->name, node->lineno);
	return InsertSymbol(activeScope, node->name, Undetermined, FunctionSym, node->lineno, NULL);
}

static SymbolEntryRec *UndeclaredVariableError(ScopeEntryRec *activeScope, TreeNode *node) //check
{
	Error = TRUE;
	fprintf(listing, "Error: undeclared variable \"%s\" is used at line %d\n", node->name, node->lineno);
	return InsertSymbol(activeScope, node->name, Undetermined, VariableSym, node->lineno, NULL);
}

static void handleVoidTypeVariableError(char *name, int lineno) // check
{
	fprintf(listing, "Error: The void-type variable is declared at line %d (name : \"%s\")\n", lineno, name);
	Error = TRUE;
}

static void handleArrayIndexingError(char *name, int lineno) //check
{
	fprintf(listing, "Error: Invalid array indexing at line %d (name : \"%s\"). indices should be integer\n", lineno, name);
	Error = TRUE;
}

static void handleArrayIndexingError2(char *name, int lineno)
{
	fprintf(listing, "Error: Invalid array indexing at line %d (name : \"%s\"). indexing can only allowed for int[] variables\n", lineno, name);
	Error = TRUE;
}

static void handleInvalidFunctionCallError(char *name, int lineno) //check
{
	fprintf(listing, "Error: Invalid function call at line %d (name : \"%s\")\n", lineno, name);
	Error = TRUE;
}

static void handleInvalidReturnError(int lineno) //check
{
	fprintf(listing, "Error: Invalid return at line %d\n", lineno);
	Error = TRUE;
}

static void handleInvalidAssignmentError(int lineno)
{
	fprintf(listing, "Error: invalid assignment at line %d\n", lineno);
	Error = TRUE;
}

static void handleInvalidOperationError(int lineno)  // check
{
	fprintf(listing, "Error: invalid operation at line %d\n", lineno);
	Error = TRUE;
}

static void handleInvalidConditionError(int lineno)
{
	fprintf(listing, "Error: invalid condition at line %d\n", lineno);
	Error = TRUE;
}



static void traverseTree(TreeNode *t, void (*preProc)(TreeNode *), void (*postProc)(TreeNode *))
{
	if (t != NULL)
	{
		preProc(t);
		int i;
		for (i = 0; i < MAXCHILDREN; i++) traverseTree(t->child[i], preProc, postProc);
		postProc(t);

		traverseTree(t->sibling, preProc, postProc);
	}
}

static void enterScope(TreeNode *t)
{
	if (t->scope != NULL) activeScope = t->scope;
}
static void exitScope(TreeNode *t)
{
	if (t->scope != NULL) activeScope = t->scope->parentScope;
}

static void addTreeNode(TreeNode *t)
{
	switch (t->kind)
	{
		case VarDecl:
		{
			if (t->type == Void || t->type == VoidArray) handleVoidTypeVariableError(t->name, t->lineno);
			SymbolEntryRec *symbol = SearchSymbolInScope(activeScope, t->name);
			if (symbol != NULL) handleRedefinitionError(t->name, t->lineno, symbol);
			InsertSymbol(activeScope, t->name, t->type, VariableSym, t->lineno, t);
			break;
		}
		case FuncDecl:
		{
			SymbolEntryRec *symbol = SearchSymbolInScope(rootScope, t->name);
			if (symbol != NULL) handleRedefinitionError(t->name, t->lineno, symbol);
			InsertSymbol(activeScope, t->name, t->type, FunctionSym, t->lineno, t);
			activeScope = t->scope = InsertScope(t->name, activeScope, t);
			break;
		}
		case Params:
		{
			if (t->conflict == TRUE) break;
			
			if (t->type == Void || t->type == VoidArray)
			{
				handleVoidTypeVariableError(t->name, t->lineno);
				break;
			}

			SymbolEntryRec *symbol = SearchSymbolInScope(activeScope, t->name);
			if (symbol != NULL) handleRedefinitionError(t->name, t->lineno, symbol);
			InsertSymbol(activeScope, t->name, t->type, VariableSym, t->lineno, t);
			break;
		}
		case CompStmt:
		{
			if (t->conflict != TRUE) t->scope = activeScope = InsertScope(NULL, activeScope, activeScope->functionNode);
			break;
		}
		case CallExpr:
		{
			SymbolEntryRec *functionNode = SearchSymbolByKind(rootScope, t->name, FunctionSym);
			if (functionNode == NULL) functionNode = UndeclaredFunctionError(rootScope, t);
			else
				InsertSymbolIntoScope(rootScope, t->name, t->lineno);
			break;
		}
		case VarAccessExpr:
		{
			SymbolEntryRec *symbol = SearchSymbolByKind(activeScope, t->name, VariableSym);
			if (symbol == NULL) symbol = UndeclaredVariableError(activeScope, t);
			else
				InsertSymbolIntoScope(activeScope, t->name, t->lineno);
			break;
		}
		case IfStmt:
		case WhileStmt:
		case OpExpr:
		case ConstExpr:
		case ReturnStmt:
		case AssignExpr:
			break;
		default: 
			break;
	}
}


void buildSymtab(TreeNode *syntaxTree)
{
	rootScope = InsertScope("global", NULL, NULL);
	activeScope = rootScope;

	TreeNode *input = newTreeNode(FuncDecl);
	input->lineno = 0;
	input->type = Integer;
	input->name = copyString("input");
	input->child[0] = newTreeNode(Params);
	input->child[0]->lineno = 0;
	input->child[0]->type = Void;
	input->child[0]->conflict = TRUE;

	TreeNode *output = newTreeNode(FuncDecl);
	output->lineno = 0;
	output->type = Void;
	output->name = copyString("output");
	TreeNode *param = newTreeNode(Params);
	param->lineno = 0;
	param->type = Integer;
	param->name = copyString("value");
	output->child[0] = param;

	InsertSymbol(rootScope, input->name, input->type, FunctionSym, input->lineno, input);
	InsertSymbol(rootScope, output->name, output->type, FunctionSym, output->lineno, output);
	ScopeEntryRec *outputScope = InsertScope("output", rootScope, output);
	InsertSymbol(outputScope, param->name, param->type, VariableSym, param->lineno,param);

	traverseTree(syntaxTree, addTreeNode, exitScope);

	if (TraceAnalyze)
	{
		DisplaySymbolTable(listing, rootScope);
	}
}

static void checkTreeNode(TreeNode *t)
{
	switch (t->kind)
	{
		case IfStmt:
		case WhileStmt:
		{
			if (t->child[0] == NULL || t->child[0]->type != Integer) 
				handleInvalidConditionError(t->lineno);
			break;
		}
		case ReturnStmt:
		{
			if (
				(t->child[0] != NULL && t->child[0]->type != activeScope->functionNode->type)
				||
				(t->child[0] == NULL && activeScope->functionNode->type != Void)
			) handleInvalidReturnError(t->lineno);
			break;
		}
		case AssignExpr:
		{
			t->type = t->child[0]->type;
			if ((t->child[0]) == NULL || (t->child[1]) == NULL || ((t->child[0])->type) != ((t->child[1])->type))
			{
					handleInvalidAssignmentError(t->lineno);
			}
			break;
		}
		
		case OpExpr:
		{
			t->type = t->child[0]->type;
			if ((t->child[0]) == NULL || (t->child[1]) == NULL)
			{
					handleInvalidOperationError(t->lineno);
					t->type = None;
			}
			else if (((t->child[0])->type) != ((t->child[1])->type))
			{
				if((t->child[0])->type != None && (t->child[1])->type != None)
				handleInvalidOperationError(t->lineno);
				t->type = None;
			}
			
			else if((t->child[0])->type != Integer ||  (t->child[1])->type != Integer)
			{	
				if((t->child[0])->type != None && (t->child[1])->type != None)
				{
				handleInvalidOperationError(t->lineno);
				}
			}
			break;
		}
		case CallExpr:
		{
			SymbolEntryRec *function = SearchSymbolByKind(rootScope, t->name, FunctionSym);
			if (function->status == undeclared)
			{
				handleInvalidFunctionCallError(t->name, t->lineno);
				t->type = function->type;
				break;
			}
			TreeNode *paramNode = function->node->child[0];
			TreeNode *argNode = t->child[0];

			if (paramNode->type == Void) 
			{
				paramNode = NULL;
				if (argNode != NULL && argNode->type == Void) 
					argNode = NULL;
			}

			while (paramNode != NULL && argNode != NULL)
			{
				if ((paramNode->type != argNode->type)) 
					handleInvalidFunctionCallError(t->name, t->lineno);

				paramNode = paramNode->sibling;
				argNode = argNode->sibling;
			}

			if (paramNode != NULL || argNode != NULL) 
				handleInvalidFunctionCallError(t->name, t->lineno);
				
			t->type = function->type;
			break;
		}
		case VarAccessExpr:
		{
			SymbolEntryRec *symbol = SearchSymbolByKind(activeScope, t->name, VariableSym);
			if (symbol->status == undeclared)
			{
				t->type = symbol->type;
				break;
			}
			if (t->child[0] != NULL)
			{
				if (symbol->type != IntegerArray)
					handleArrayIndexingError2(t->name, t->lineno);
				if (t->child[0]->type != Integer) 
					handleArrayIndexingError(t->name, t->lineno);
				
				t->type = Integer;
			}
			else
				t->type = symbol->type;
			break;
		}
		case ConstExpr:
		{
			t->type = Integer;
			break;
		}
		case Params:
		case FuncDecl:
		case VarDecl:
		case CompStmt:
		default: 
			break;
	}
}

void typeCheck(TreeNode *syntaxTree) {
	traverseTree(syntaxTree, enterScope, checkTreeNode);
}
