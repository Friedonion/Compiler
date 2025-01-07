/****************************************************/
/* File: util.c                                     */
/* Utility function implementation                  */
/* for the TINY compiler                            */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "util.h"

#include "globals.h"

/* Procedure printToken prints a token
 * and its lexeme to the listing file
 */
 
 static char* TypetoString(NodeType type) {
    if (type == Integer) return "int";
    else if (type == Undetermined) return "undetermined";
    else if (type == IntegerArray) return "int[]";
    else if (type == Void) return "void";
    else if (type == VoidArray) return "void[]";
    else return "unknown";
}


void printToken(TokenType token,
    const char * tokenString) {
    switch (token) {
        case IF:
        case ELSE:
        case WHILE:
        case RETURN:
        case INT:
        case VOID:
            fprintf(listing, "reserved word: %s\n", tokenString);
            break;
        case ASSIGN:
            fprintf(listing, "=\n");
            break;
        case EQ:
            fprintf(listing, "==\n");
            break;
        case NE:
            fprintf(listing, "!=\n");
            break;
        case LT:
            fprintf(listing, "<\n");
            break;
        case LE:
            fprintf(listing, "<=\n");
            break;
        case GT:
            fprintf(listing, ">\n");
            break;
        case GE:
            fprintf(listing, ">=\n");
            break;
        case PLUS:
            fprintf(listing, "+\n");
            break;
        case MINUS:
            fprintf(listing, "-\n");
            break;
        case TIMES:
            fprintf(listing, "*\n");
            break;
        case OVER:
            fprintf(listing, "/\n");
            break;
        case LPAREN:
            fprintf(listing, "(\n");
            break;
        case RPAREN:
            fprintf(listing, ")\n");
            break;
        case LBRACE:
            fprintf(listing, "[\n");
            break;
        case RBRACE:
            fprintf(listing, "]\n");
            break;
        case LCURLY:
            fprintf(listing, "{\n");
            break;
        case RCURLY:
            fprintf(listing, "}\n");
            break;
        case SEMI:
            fprintf(listing, ";\n");
            break;
        case COMMA:
            fprintf(listing, ",\n");
            break;
        case ENDFILE:
            fprintf(listing, "EOF\n");
            break;

        case NUM:
            fprintf(listing, "NUM, val= %s\n", tokenString);
            break;
        case ID:
            fprintf(listing, "ID, name= %s\n", tokenString);
            break;
        case ERROR:
            fprintf(listing, "ERROR: %s\n", tokenString);
            break;
        default:
            fprintf(listing, "Unknown token: %d\n", token);
    }
}

/* Function newStmtNode creates a new statement
 * node for syntax tree construction
 */
 

TreeNode * newTreeNode(NodeKind kind) {
    TreeNode * t = (TreeNode * ) malloc(sizeof(TreeNode));
    if (t == NULL) {
        fprintf(listing, "Out of memory error at line %d\n", lineno);
        return t;
    }

    int i;
    for (i = 0; i < MAXCHILDREN; i++) t -> child[i] = NULL;
    t -> sibling = NULL;
    t -> lineno = lineno;
    t -> kind = kind;
    t -> type = None;
    t -> name = NULL;
    t -> val = 0;
    t -> conflict = FALSE;
    t -> token = -1;

    return t;
}

/* Function copyString allocates and makes a new
 * copy of an existing string
 */
char * copyString(char * s) {
    int n;
    char * t;
    if (s == NULL) return NULL;
    n = strlen(s) + 1;
    t = malloc(n);
    if (t == NULL) fprintf(listing, "Out of memory error at line %d\n", lineno);
    else
        strcpy(t, s);
    return t;
}

/* Variable indentno is used by printTree to
 * store current number of spaces to indent
 */
static int indentno = 0;

#define INDENT indentno += 2
#define UNINDENT indentno -= 2

static void printSpaces(void) {
    int i;
    for (i = 0; i < indentno; i++) fprintf(listing, " ");
}


/* procedure printTree prints a syntax tree to the
 * listing file using indentation to indicate subtrees
 */
void printTree(TreeNode * tree) {
    int i;
    INDENT;
    while (tree != NULL) {
        printSpaces();
        switch (tree -> kind) {
            case VarDecl:
                fprintf(listing, "Variable Declaration: name = %s, type = %s\n", tree -> name, TypetoString(tree -> type));
                break;
            case FuncDecl:
                fprintf(listing, "Function Declaration: name = %s, return type = %s\n", tree -> name, TypetoString(tree -> type));
                break;
            case Params:
                if (tree -> conflict == TRUE) fprintf(listing, "Void Parameter\n");
                else
                    fprintf(listing, "Parameter: name = %s, type = %s\n", tree -> name, TypetoString(tree -> type));
                break;
            case CompStmt:
                fprintf(listing, "Compound Statement:\n");
                break;
            case IfStmt:
                if (tree -> conflict == TRUE) fprintf(listing, "If-Else Statement:\n");
                else
                    fprintf(listing, "If Statement:\n");
                break;
            case WhileStmt:
                fprintf(listing, "While Statement:\n");
                break;
            case ReturnStmt:
                if (tree -> conflict == TRUE) fprintf(listing, "Return Statement\n");
                else
                    fprintf(listing, "Non-value Return Statement:\n");
                break;
            case AssignExpr:
                fprintf(listing, "Assign:\n");
                break;
            case VarAccessExpr:
                fprintf(listing, "Variable: name = %s\n", tree -> name);
                break;
            case OpExpr:
                fprintf(listing, "Op: ");
                printToken(tree -> token, "");
                break;
            case ConstExpr:
                fprintf(listing, "Const: %d\n", tree -> val);
                break;
            case CallExpr:
                fprintf(listing, "Call: function name = %s\n", tree -> name);
                break;
            default:
                fprintf(listing, "Unknown Node Kind : %d (%x)\n", tree -> kind, tree -> kind);
                break;
        }

        for (i = 0; i < MAXCHILDREN; i++) printTree(tree -> child[i]);
        tree = tree -> sibling;
    }
    UNINDENT;
}
