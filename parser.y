%{
#include <cstdio>
#include "pcl_lexer.hpp"
#include "ast.hpp"
#include <string>
std::map<std::string, Const*> globals; // map variable names to values
std::map<std::string, Type*> declared; // map variable names to values

%}

%define parse.error verbose
%expect 1

%token<var> T_id
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
%token<numi> T_iconst
%token<numd> T_rconst
%token<var> T_sconst
%token<ch> T_cconst


%nonassoc '=' '<' '>' "<=" ">=" "<>"
%left '+' '-' "or"
%left '*' '/' "div" "mod" "and"
%nonassoc "not"
%nonassoc UMINUS UPLUS
%nonassoc '^'
%nonassoc '@'
%nonassoc BRACKETS

%union{
	Body* body;
	Stmt* stmt;
	ExprList* exprList;
	DeclList* declList;
	StmtList* stmtList;
	Expr* expr;
	Type* type;
	LValue* lvalue;
	std::string* var;
	int numi;
	double numd;
	char ch;
}

%type<body> program body
%type<stmtList> block mult_stmts
%type<declList> mult_locals local var_decl mult_ids args mult_formals formal
%type<exprList> params mult_exprs
%type<stmt> stmt
%type<expr> expr r_value call
%type<lvalue> l_value_ref l_value
%type<type> type full_type

%%

program:
  "program" T_id ';' body '.' {std::cout << "AST: " << *$4 << std::endl; /*TODO $$ = new Program($2,$4);*/$4->run();
	std::cout<<"DECLARED: "<<declared<<std::endl<<"GLOBALS: "<<globals<<std::endl;}
;

body:
  mult_locals block {$$=new Body($1,$2);}
;

mult_locals:
 /* nothing */ {$$ = new DeclList();}
| mult_locals local { $1->merge($2); }
;

local:
  "var" var_decl {$$ = $2;}
| "label" mult_ids ';' {$2->toLabel(); $$=$2;}
| header ';' body ';' {/*TODO $1->body($3); $$=$1;*/}
| "forward" header ';' {/*TODO $$ = new Forward($2);*/}
;

var_decl:
  mult_ids ':' type ';' {$1->toVar($3); $$ = $1;}
| var_decl mult_ids ':' type ';' {$2->toVar($4); $1->merge($2); $$=$1;}
;

mult_ids:
  T_id { $$ = new DeclList(new Decl(*$1));}
| mult_ids ',' T_id {$1->append(new Decl(*$3)); $$=$1;}
;

type:
  "integer" {$$=INTEGER::getInstance();}
| "real"  {$$ = REAL::getInstance(); }
| "boolean" {$$ =BOOLEAN::getInstance(); }
| "char" {$$ = CHARACTER::getInstance(); }
| "array" '[' T_iconst ']' "of" full_type {$$ = new ArrType($3,$6);}
| "array" "of" full_type {$$ = new ArrType($3);}
| '^' full_type {$$ = new PtrType($2);}
;

full_type:
"integer" {$$=INTEGER::getInstance();}
| "real"  {$$ = REAL::getInstance(); }
| "boolean" {$$ =BOOLEAN::getInstance(); }
| "char" {$$ = CHARACTER::getInstance(); }
| "array" '[' T_iconst ']' "of" full_type {$$ = new ArrType($3,$6);}
| '^' full_type {$$ = new PtrType($2);}
;

header:
  "procedure" T_id '(' args ')' {/*TODO $$ = new Function($2,$4,new Type("void"));*/}
| "function" T_id '(' args ')' ':' type {/*TODO $$ = new Function($2,$4,$6);*/}
;

args:
/*nothing*/ {$$ = new DeclList();}
| mult_formals {$$ = $1;}
;

mult_formals:
  formal {$$ = $1;}
| mult_formals ';' formal {$1->merge($3);}
;

formal:
  "var" mult_ids ':' type {$2->toFormal($4,true); $$=$2;}
| mult_ids ':' type {$1->toFormal($3,false); $$ = $1;}
;

block:
  "begin" mult_stmts "end" {$$=$2;}
;

mult_stmts:
  stmt {$$ = new StmtList($1);}
| mult_stmts ';' stmt { $1->append($3);}
;

stmt:
/*nothing*/ {$$ = new Stmt();}
| l_value ":=" expr {$$ = new Let($1,$3);}
| block {$$= $1;}
| call {$$= new Stmt();/*TODO $$= new Stmt($1);*/ /*call can be a statement only if it is a proc call*/}
| "if" expr "then" stmt "else" stmt {$$ = new If($2,$4,$6);}
| "if" expr "then" stmt {$$ = new If($2,$4,nullptr);}
| "while" expr "do" stmt {$$ = new While($2, $4);}
| T_id ':' stmt { $$=$3;/*TODO $3->target($1); $$=$3; */}
| "goto" T_id { $$=new Stmt();/*TODO $$ = new Goto($2);*/ }
| "return" {$$=new Stmt();/*TODO $$ = new Return();*/}
| "new" '[' expr ']' l_value {$$ = new New($5,$3);}
| "new" l_value {$$=new New($2,nullptr);}
| "dispose" '[' ']' l_value {$$ = new DisposeArr($4);}
| "dispose" l_value {$$ = new Dispose($2);}
;

expr:
  l_value {$$ = $1;}
| r_value {$$ = $1;}

l_value_ref:
  T_id {$$ = new Id(*$1);}
| "result" {$$ = new Id("result");}
| T_sconst {$$ = new Sconst(*$1);}
| l_value_ref '[' expr ']' %prec BRACKETS {$$ = new Brackets($1,$3);}
| '(' l_value ')' {$$ = $2;}

l_value:
  expr '^' {$$ = new Dereference($1);}
| T_id {$$ = new Id(*$1);}
| "result" {$$ = new Id("result");}
| T_sconst {$$ = new Sconst(*$1);}
| l_value '[' expr ']' %prec BRACKETS {$$ = new Brackets($1,$3);}
| '(' l_value ')'{$$ = $2;}
;

r_value:
  T_rconst {$$ = new Rconst($1);}
| T_iconst {$$ = new Iconst($1);}
| T_cconst {$$ = new Cconst($1);}
| "true" {$$ = new Bconst("true");}
| "false" {$$ = new Bconst("false");}
| '(' r_value ')' {$$ = $2;}
| "nil" {$$ = new Pconst(); /*pointer constant*/}
| call {$$ = $1;}
| '@' l_value_ref {$$ = new Reference($2);}
| expr '+' expr {$$ = new Op($1,"+",$3);}
| expr '-' expr {$$ = new Op($1,"-",$3);}
| expr '*' expr {$$ = new Op($1,"*",$3);}
| expr '/' expr {$$ = new Op($1,"/",$3);}
| expr "<>" expr {$$ = new Op($1,"<>",$3);}
| expr "<=" expr {$$ = new Op($1,"<=",$3);}
| expr ">=" expr {$$ = new Op($1,">=",$3);}
| expr '=' expr {$$ = new Op($1,"=",$3);}
| expr '>' expr {$$ = new Op($1,">",$3);}
| expr '<' expr {$$ = new Op($1,"<",$3);}
| expr "div" expr {$$ = new Op($1,"div",$3);}
| expr "mod" expr {$$ = new Op($1,"mod",$3);}
| expr "and" expr {$$ = new Op($1,"and",$3);}
| expr "or" expr {$$ = new Op($1,"or",$3);}
| "not" expr {$$ = new Op("not",$2);}
| '+' expr %prec UPLUS {$$ = new Op("+",$2);}
| '-' expr %prec UMINUS {$$ = new Op("-",$2);}
;

call:
  T_id '('params')' {/*TODO $$ = new Call($1,$3);*/}
;

params:
/* nothing */ { $$ = new ExprList();}
|mult_exprs {$$ = $1;}
;

mult_exprs:
  expr {$$ = new ExprList($1);}
| mult_exprs ',' expr { $1->append($3); $$ = $1;}
;

%%

int main() {
  int result = yyparse();
  if (result == 0) printf("Success.\n");
  return result;
}
