/* ------------------------------------------
pcl_lexer.l
Lexical parser for pcl compiler in flex
------------------------------------------ */
%x C_COMMENT
%x STRING
%x CHARACTER
%{
	#include <cstdio>
	#include <cstdlib>
	#include "ast.hpp"
	#include "pcl_lexer.hpp"
	#include "parser.hpp"
	#include <string>
%}

%option noyywrap


%{
	char c;
	std::string *s;
	bool added;
	extern char msg[100];
	extern char linebuf[500];
	extern struct symbol_loc location;
	#define YY_USER_ACTION \
    location.first_line = location.last_line; \
    location.first_column = location.last_column; \
    for(int i = 0; yytext[i] != '\0'; i++) { \
        if(yytext[i] == '\n') { \
            location.last_line++; \
            location.last_column = 0; \
        } \
        else { \
            location.last_column++; \
        } \
    }
%}
L [a-zA-Z]
D [0-9]
E \\(n|t|r|\'|\"|0|\\)
W [ \t\r\n]
%%
\n.* { strncpy(linebuf, yytext+1, sizeof(linebuf)); /* save the next line' */
       yyless(1); /* give back all but the \n to rescan */}
"var" {return T_var;}
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

\" {BEGIN(STRING); s=new std::string("");/*"*/}
<STRING>\\n {s->push_back('\n');}
<STRING>\\t {s->push_back('\t');}
<STRING>\\r {s->push_back('\r');}
<STRING>\\0 {s->push_back('\0');}
<STRING>\\\' {s->push_back('\'');}
<STRING>\\\" {s->push_back('\"');/*"*/}
<STRING>\\\\ {s->push_back('\\');}
<STRING>[^'\"] {s->push_back(yytext[0]);/*'*/}
<STRING>\" {BEGIN(INITIAL);yylval.var=s; return T_sconst;/*"*/}
<STRING>.$ {yyerror("Unterminated string.");}
<STRING>. {sprintf(msg,"Invalid character '%c' while parsing string.",yytext[0]);yyerror(msg);}


\' {BEGIN(CHARACTER); added=false;/*'*/}
<CHARACTER>\\n {if(!added){c='\n'; added=true;}else {yyerror("Too long for character.");}}
<CHARACTER>\\t {if(!added){c='\t'; added=true;}else {yyerror("Too long for character.");}}
<CHARACTER>\\r {if(!added){c='\r'; added=true;}else {yyerror("Too long for character.");}}
<CHARACTER>\\0 {if(!added){c='\0'; added=true;}else {yyerror("Too long for character.");}}
<CHARACTER>\\\' {if(!added){c='\''; added=true;}else {yyerror("Too long for character.");}}
<CHARACTER>\\\" {if(!added){c='\"'; added=true;}else {yyerror("Too long for character.");}/*"*/}
<CHARACTER>\\\\ {if(!added){c='\\'; added=true;}else {yyerror("Too long for character.");}}
<CHARACTER>[^'\"] {if(!added){c=yytext[0]; added=true;}else {yyerror("Too long for character.");}/*'*/}
<CHARACTER>\' {BEGIN(INITIAL);yylval.ch=c; return T_cconst;/*'*/}
<CHARACTER>. {sprintf(msg,"Invalid character '%c'.",yytext[0]);yyerror(msg); }

":=" {return T_assign;}
"<>" {return T_dt;}
"<=" {return T_lt;}
">=" {return T_gt;}
[\[\]()+/\-*:;.<>@^=,] {return yytext[0];}
{W}+  {/* nothing */}
"(*"            { BEGIN(C_COMMENT); }
<C_COMMENT>"*)" { BEGIN(INITIAL); }
<C_COMMENT>\n { }
<C_COMMENT>.    { }
. {
	fprintf(stderr, "Illegal character with code %d\n",yytext[0]);
	exit(1);
}
%%

void yyerror(const char *msg) {
	fprintf(stderr, "Line %d: %s\n%s\n",
		location.first_line,msg,linebuf);
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
