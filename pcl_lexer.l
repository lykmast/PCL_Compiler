%option noyywrap
%x C_COMMENT
%{
	#include <cstdio>
	#include <cstdlib>
	#include "ast.hpp"
	#include "pcl_lexer.hpp"
	#include "parser.hpp"
	#include <string>
%}

L [a-zA-Z]
D [0-9]
E \\(n|t|r|\'|\"|0|\\)?
W [ \t\r\n]
%%
"var" {return T_var;/*'*/}
"integer" {return T_integer;}
"boolean" {return T_boolean;}
"char" {return T_char;}
"real" {return T_real;}
"array" {return T_array;}
"of" {return T_of;}
"program" {return T_program;}
"procedure" {return T_procedure;}
"forward" {return T_forward;}
"function" {return T_function;}
"begin" {return T_begin;}
"end" {return T_end;}
"if" {return T_if;}
"then" {return T_then;}
"else" {return T_else;}
"while" {return T_while;}
"do" {return T_do;}
"goto" {return T_goto;}
"label" {return T_label;}
"return" {return T_return;}
"not" {return T_not;}
"and" {return T_and;}
"or" {return T_or;}
"div" {return T_div;}
"mod" {return T_mod;}
"true" {return T_true;}
"false" {return T_false;}
"nil" {return T_nil;}
"dispose" {return T_dispose;}
"new" {return T_new;}
"result" {return T_result;}
{L}(_|{L}|{D})* {std::string *s=new std::string(yytext);yylval.var=s;return T_id;}
{D}+\.{D}+((e|E)(\-|\+)?{D}+)? {yylval.numd=atof(yytext);return T_rconst;}
{D}+ {yylval.numi=atoi(yytext);return T_iconst;}
\"([^\'\\\"]|{E})*\" {std::string* s=new std::string(yytext);yylval.var=s;return T_sconst;/*"*/}
\'([^\'\\\"]|{E})\'	{yylval.ch=yytext[0];return T_cconst;/*'*/}
":=" {return T_assign;}
"<>" {return T_dt;}
"<=" {return T_lt;}
">=" {return T_gt;}
[\[\]()+/\-*:;.<>@^=,] {return yytext[0];}
{W}+  {/* nothing */}
"(*"            { BEGIN(C_COMMENT); }
<C_COMMENT>"*)" { BEGIN(INITIAL); }
<C_COMMENT>.    { }
<C_COMMENT>\n { }
. {
	fprintf(stderr, "Illegal character with code %d\n",yytext[0]);
	exit(1);
}
%%

void yyerror(const char *msg) {
  fprintf(stderr, "%s\n", msg);
  exit(1);
}

// #ifdef yyFlexLexer
// int main () {
// 	yyFlexLexer lexer;
// 	int token;
// 	do {
// 		token = lexer.yylex();
// 		printf("token=%d, lexeme=\"%s\"\n", token, lexer.YYText());
// 	} while (token != T_eof);
// }
// #else
// int main () {
// 	int token;
// 	do {
// 		token = yylex();
// 		printf("token=%d, lexeme=\"%s\"\n", token, yytext);
// 	} while (token != T_eof);
// }
// #endif
