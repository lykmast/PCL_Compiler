%{
#include <cstdio>
#include "pcl_lexer.hpp"
%}

%define parse.error verbose
%expect 1

%token T_id
%token T_var "var"
%token T_integer "integer"
%token T_boolean "boolean"
%token T_char "char"
%token T_real "real"
%token T_array "array"
%token T_of "of"
%token T_program "program"
%token T_procedure "procedure"
%token T_forward "forward"
%token T_function "function"
%token T_begin "begin"
%token T_end "end"
%token T_if "if"
%token T_then "then"
%token T_else "else"
%token T_while "while"
%token T_do "do"
%token T_goto "goto"
%token T_label "label"
%token T_return "return"
%token T_not "not"
%token T_and "and"
%token T_or "or"
%token T_div "div"
%token T_mod "mod"
%token T_true "true"
%token T_false "false"
%token T_nil "nil"
%token T_dispose "dispose"
%token T_new "new"
%token T_result "result"
%token T_assign ":="
%token T_dt "<>"
%token T_lt "<="
%token T_gt ">="
%token T_iconst
%token T_rconst
%token T_sconst
%token T_cconst


%nonassoc '=' '<' '>' "<=" ">=" "<>"
%left '+' '-' "or"
%left '*' '/' "div" "mod" "and"
%nonassoc "not"
%nonassoc UMINUS UPLUS
%nonassoc '^'
%nonassoc '@'
%nonassoc BRACKETS



%%

program:
  "program" T_id ';' body '.'
;

body:
  block
| local body
;

local:
  "var" var_decl
| "label" mult_ids ';'
| header ';' body ';'
| "forward" header ';'
;

var_decl:
  mult_ids ':' type ';'
| mult_ids ':' type ';' var_decl
;

mult_ids:
  T_id
| T_id ',' mult_ids
;

type:
  "integer"
| "real"
| "boolean"
| "char"
| "array" '[' T_iconst ']' "of" type
| "array" "of" type
| '^' type
;

header:
  "procedure" T_id '(' args ')'
| "function" T_id '(' args ')' ':' type
;

args:

| mult_formals
;

mult_formals:
  formal
| formal ';' mult_formals
;

formal:
  "var" mult_ids ':' type
| mult_ids ':' type
;

block:
  "begin" mult_stmts "end"
;

mult_stmts:
  stmt
| stmt ';' mult_stmts
;

stmt:

| l_value ":=" expr
| block
| call
| "if" expr "then" stmt "else" stmt
| "if" expr "then" stmt
| "while" expr "do" stmt
| T_id ':' stmt
| "goto" T_id
| "return"
| "new" '[' expr ']' l_value
| "new" l_value
| "dispose" '[' ']' l_value
| "dispose" l_value
;

expr:
  l_value
| r_value

l_value_ref:
  T_id;
| "result"
| T_sconst
| l_value_ref '[' expr ']' %prec BRACKETS
| '(' l_value ')'

l_value:
  expr '^'
| T_id;
| "result"
| T_sconst
| l_value '[' expr ']' %prec BRACKETS
| '(' l_value ')'
;

r_value:
  T_rconst
| T_iconst
| T_cconst
| "true"
| "false"
| '(' r_value ')'
| "nil"
| call
| '@' l_value_ref
| expr '+' expr
| expr '-' expr
| expr '*' expr
| expr '/' expr
| expr "<>" expr
| expr "<=" expr
| expr ">=" expr
| expr '=' expr
| expr '>' expr
| expr '<' expr
| expr "div" expr
| expr "mod" expr
| expr "and" expr
| expr "or" expr
| "not" expr
| '+' expr %prec UPLUS
| '-' expr %prec UMINUS
;

call:
  T_id '('params')'
;

params:

|mult_exprs
;

mult_exprs:
  expr
| expr ',' mult_exprs
;

%%

int main() {
  int result = yyparse();
  if (result == 0) printf("Success.\n");
  return result;
}
