%option noyywrap

%{
	#define T_eof 0
	#define T_id 1001
	// #define T_cconst 1002
	#define T_var 1003
	#define T_integer 1004
	#define T_boolean 1005
	#define T_char 1006
	#define T_real 1007
	#define T_array 1008
	#define T_of 1009
	#define T_program 1010
	#define T_procedure 1011
	#define T_forward 1012
	#define T_function 1013
	#define T_begin 1014
	#define T_end 1015
	#define T_if 1016
	#define T_then 1017
	#define T_else 1018
	#define T_while 1019
	#define T_do 1020
	#define T_goto 1021
	#define T_label 1022
	#define T_return 1023
	#define T_not 1024
	#define T_and 1025
	#define T_or 1026
	#define T_div 1027
	#define T_mod 1028
	#define T_true 1029
	#define T_false 1030
	#define T_nil 1031
	#define T_dispose 1032
	#define T_new 1033
	#define T_result 1034
	#define T_assign 1035
	#define T_dt 1036
	#define T_lt 1037
	#define T_gt 1038
	#define T_iconst 1039
	#define T_rconst 1040
	#define T_sconst 1041
	#define T_cconst 1042
%}
L [a-zA-Z]
D [0-9]
E \\(n|t|r|\'|\"|0)?
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
{L}(_|{L}|{D})* {return T_id;}
(\-|\+)?{D}+\.{D}+(e(\-|\+)?{D}+)? {return T_rconst;}
(\-|\+)?{D}+ {return T_iconst;}
\"([^\"\\]|{E})*\" {return T_sconst;/*"*/}
\'([^\'\\]|{E})\'	{return T_cconst;/*'*/}
":=" {return T_assign;}
"<>" {return T_dt;}
"<=" {return T_lt;}
">=" {return T_gt;}
[\[\]()+/\-*:;.<>@^=] {return yytext[0];}
{W}+  {/* nothing */}
"(*"[.\n]*"*)" {/* nothing' */}
. {
	fprintf(stderr, "Illegal character with code %d\n",yytext[0]);
	exit(1);
}
%%
#ifdef yyFlexLexer
int main () {
	yyFlexLexer lexer;
	int token;
	do {
		token = lexer.yylex();
		printf("token=%d, lexeme=\"%s\"\n", token, lexer.YYText());
	} while (token != T_eof);
}
#else
int main () {
	int token;
	do {
		token = yylex();
		printf("token=%d, lexeme=\"%s\"\n", token, yytext);
	} while (token != T_eof);
}
#endif
