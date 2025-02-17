/****************************************************/
/* File: tiny.l                                     */
/* Lex specification for TINY                       */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

%{
#include "globals.h"
#include "util.h"
#include "scan.h"
/* lexeme of identifier or reserved word */
char* tokenString;
%}

digit       [0-9]
number      {digit}+
letter      [a-zA-Z]
identifier	{letter}({letter}|{digit})* 
%{ 
/* identifier  {letter}+ */
%}
newline     \n
whitespace  [ \t]+

%%

"if"         { return IF;}
"else"       { return ELSE;}
"while"      { return WHILE;}
"return"     { return RETURN;}
"int"        { return INT;}
"void"       { return VOID;}
"="          { return ASSIGN;}
"=="         { return EQ;}
"!="         { return NE;}
"<"          { return LT;}
"<="         { return LE;}
">"          { return GT;}
">="         { return GE;}
"+"          { return PLUS;}
"-"          { return MINUS;}
"*"          { return TIMES;}
"/"          { return OVER;}
"("          { return LPAREN;}
")"          { return RPAREN;}
"["          { return LBRACE;}
"]"          { return RBRACE;}
"{"          { return LCURLY;}
"}"          { return RCURLY;}
";"          { return SEMI;}
","          { return COMMA;}
{number}     { return NUM;}
{identifier} { return ID;}
{newline}    { lineno++;}
{whitespace} { /* skip whitespace */}
"/*"         {
				char c;
				int end_comment_ = 0;
				int end_comment = 0;
				do
				{
					c = input();

					// if (c == EOF || c == '\0') return ERROR;
					if ( c == EOF || c == '\0' ) return ENDFILE;
					if (c == '\n') lineno++;
					if (end_comment_ && c == '/') end_comment = 1;
					if (c == '*') end_comment_ = 1;
					else end_comment_ = 0;
				} 
				while ( !end_comment );
             }
.            { return ERROR;}
%%

TokenType getToken(void)
{ 
	static int firstTime = TRUE;
	TokenType currentToken;
	if (firstTime)
	{ 
		firstTime = FALSE;
		lineno++;
		yyin = source;
		yyout = listing;
	}
	currentToken = yylex();
  	tokenString = (char*)malloc(sizeof(char)*(strlen(yytext)+1));
  	strncpy(tokenString,yytext,strlen(yytext)+1);
	if (TraceScan) {
		fprintf(listing,"\t%d: ",lineno);
		printToken(currentToken,tokenString);
	}
	return currentToken;
}
