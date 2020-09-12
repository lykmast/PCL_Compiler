/* ------------------------------------------
parser.y
Syntax parser for pcl compiler in bison.
  Creates AST.
------------------------------------------ */
%{
#include <cstdio>
#include "pcl_lexer.hpp"
#include "ast.hpp"
#include <string>
#include <vector>

%}
%code provides{
	extern char msg[100];
	extern char linebuf[500];
	extern struct symbol_loc location;
}

%code{
	char msg[100];
	char linebuf[500];
	struct symbol_loc location;
}

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
	Program* program;
	Body* body;
	Stmt* stmt;
	ExprList* exprList;
	DeclList* declList;
	Procedure* proc;
	StmtList* stmtList;
	Expr* expr;
	TSPtr* type;
	LValue* lvalue;
	std::string* var;
	int numi;
	double numd;
	char ch;
}
%type<program> program;
%type<body> body
%type<stmtList> block mult_stmts
%type<declList> mult_locals local var_decl mult_ids args mult_formals formal
%type<exprList> params mult_exprs
%type<stmt> stmt proc_call
%type<expr> expr r_value fun_call
%type<lvalue> l_value_ref l_value
%type<type> type full_type
%type<proc> header
%%

program:
  "program" T_id ';' body '.'
  		{$$=new Program(*$2,$4);$$->add_parse_info(location, linebuf);
	    $$->sem();
	     /* std::cout<<"before sem: "<<*$$<<std::endl;
	    std::cout<<"after sem: "<<*$$<<std::endl;
	    // fflush(stdin); */
	    $$->cgen();}
;

// {std::cout << "AST: " << *$4 << std::endl; $$ = new Program($4);$$->add_parse_info(location, linebuf);std::cout<<"between sem and run"<<std::endl; std::cout << "AST: " << *$4 << std::endl; $4->run();
//  std::cout << "AST: " << *$4 << std::endl; }

body:
  mult_locals block {$$=new Body($1,$2);$$->add_parse_info(location, linebuf);}
;

mult_locals:
 /* nothing */ {$$ = new DeclList();$$->add_parse_info(location, linebuf);}
| mult_locals local { $1->merge($2); }
;

local:
  "var" var_decl {$$ = $2;}
| "label" mult_ids ';' {$2->toLabel(); $$=$2;}
| header ';' body ';' {$1->add_body($3); $$=new DeclList($1);$$->add_parse_info(location, linebuf);}
| "forward" header ';' {$$ = new DeclList($2);$$->add_parse_info(location, linebuf);}
;

var_decl:
  mult_ids ':' type ';' {$1->toVar(*$3); $$ = $1;}
| var_decl mult_ids ':' type ';' {$2->toVar(*$4); $1->merge($2); $$=$1;}
;

mult_ids:
  T_id { $$ = new DeclList(new Decl(*$1));$$->add_parse_info(location, linebuf);}
| mult_ids ',' T_id {$1->append(new Decl(*$3)); $$=$1;}
;

type:
  "integer" {$$= INTEGER::getPtrInstance();}
| "real"  {$$ = REAL::getPtrInstance(); }
| "boolean" {$$ = BOOLEAN::getPtrInstance(); }
| "char" {$$ = CHARACTER::getPtrInstance(); }
| "array" '[' T_iconst ']' "of" full_type {$$ = new TSPtr(new ArrType($3,*$6));}
| "array" "of" full_type {$$ = new TSPtr( new ArrType(*$3));}
| '^' type {$$ = new TSPtr(new PtrType(*$2));}
;

full_type:
"integer" {$$= INTEGER::getPtrInstance();}
| "real"  {$$ = REAL::getPtrInstance(); }
| "boolean" {$$ = BOOLEAN::getPtrInstance(); }
| "char" {$$ = CHARACTER::getPtrInstance(); }
| "array" '[' T_iconst ']' "of" full_type {$$ = new TSPtr(new ArrType($3,*$6));}
| '^' full_type {$$ = new TSPtr (new PtrType(*$2));}
;

header:
  "procedure" T_id '(' args ')' {$$ = new Procedure(*$2,$4, new Body());$$->add_parse_info(location, linebuf);}
| "function" T_id '(' args ')' ':' type {$$ = new Function(*$2,$4,*$7, new Body());$$->add_parse_info(location, linebuf);}
;

args:
/*nothing*/ {$$ = new DeclList();$$->add_parse_info(location, linebuf);}
| mult_formals {$$ = $1;}
;

mult_formals:
  formal {$$ = $1;}
| mult_formals ';' formal {$1->merge($3);}
;

formal:
  "var" mult_ids ':' type {$2->toFormal(*$4,true); $$=$2;}
| mult_ids ':' type {$1->toFormal(*$3,false); $$ = $1;}
;

block:
  "begin" mult_stmts "end" {$$=$2; }
;

mult_stmts:
  stmt {$$ = new StmtList($1);$$->add_parse_info(location, linebuf);}
| mult_stmts ';' stmt { $1->append($3);}
;

stmt:
/*nothing*/ {$$ = new Stmt();$$->add_parse_info(location, linebuf);}
| l_value ":=" expr {$$ = new Let($1,$3);$$->add_parse_info(location, linebuf); }
| block {$$= $1;}
| proc_call {$$=$1; /*call can be a statement only if it is a proc call*/}
| "if" expr "then" stmt "else" stmt {$$ = new If($2,$4,$6);$$->add_parse_info(location, linebuf);}
| "if" expr "then" stmt {$$ = new If($2,$4,nullptr);$$->add_parse_info(location, linebuf);}
| "while" expr "do" stmt {$$ = new While($2, $4);$$->add_parse_info(location, linebuf);}
| T_id ':' stmt { $$=new LabelStmt(*$1, $3);$$->add_parse_info(location, linebuf);}
| "goto" T_id { $$ = new Goto(*$2);$$->add_parse_info(location, linebuf);}
| "return" {$$ = new Return();$$->add_parse_info(location, linebuf);}
| "new" '[' expr ']' l_value {$$ = new New($5,$3);$$->add_parse_info(location, linebuf);}
| "new" l_value {$$=new New($2,nullptr);$$->add_parse_info(location, linebuf);}
| "dispose" '[' ']' l_value {$$ = new DisposeArr($4);$$->add_parse_info(location, linebuf);}
| "dispose" l_value {$$ = new Dispose($2);$$->add_parse_info(location, linebuf);}
;

expr:
  l_value {$$ = $1;}
| r_value {$$ = $1;}

l_value_ref:
  T_id {$$ = new Id(*$1);$$->add_parse_info(location, linebuf);}
| "result" {$$ = new Id("result");$$->add_parse_info(location, linebuf);}
| T_sconst {$$ = new Sconst(*$1);$$->add_parse_info(location, linebuf);}
| l_value_ref '[' expr ']' %prec BRACKETS {$$ = new Brackets($1,$3);$$->add_parse_info(location, linebuf);}
| '(' l_value ')' {$$ = $2;}

l_value:
  expr '^' {$$ = new Dereference($1);$$->add_parse_info(location, linebuf);}
| T_id {$$ = new Id(*$1);$$->add_parse_info(location, linebuf);}
| "result" {$$ = new Id("result");$$->add_parse_info(location, linebuf);}
| T_sconst {$$ = new Sconst(*$1);$$->add_parse_info(location, linebuf);}
| l_value '[' expr ']' %prec BRACKETS {$$ = new Brackets($1,$3);$$->add_parse_info(location, linebuf);}
| '(' l_value ')'{$$ = $2;}
;

r_value:
  T_rconst {$$ = new Rconst($1);$$->add_parse_info(location, linebuf);}
| T_iconst {$$ = new Iconst($1);$$->add_parse_info(location, linebuf);}
| T_cconst {$$ = new Cconst($1);$$->add_parse_info(location, linebuf);}
| "true" {$$ = new Bconst(true);$$->add_parse_info(location, linebuf);}
| "false" {$$ = new Bconst(false);$$->add_parse_info(location, linebuf);}
| '(' r_value ')' {$$ = $2;}
| "nil" {$$ = new NilConst();$$->add_parse_info(location, linebuf); /*pointer constant*/}
| fun_call {$$ = $1;}
| '@' l_value_ref {$$ = new Reference($2);$$->add_parse_info(location, linebuf);}
| expr '+' expr {$$ = new Op($1,"+",$3);$$->add_parse_info(location, linebuf);}
| expr '-' expr {$$ = new Op($1,"-",$3);$$->add_parse_info(location, linebuf);}
| expr '*' expr {$$ = new Op($1,"*",$3);$$->add_parse_info(location, linebuf);}
| expr '/' expr {$$ = new Op($1,"/",$3);$$->add_parse_info(location, linebuf);}
| expr "<>" expr {$$ = new Op($1,"<>",$3);$$->add_parse_info(location, linebuf);}
| expr "<=" expr {$$ = new Op($1,"<=",$3);$$->add_parse_info(location, linebuf);}
| expr ">=" expr {$$ = new Op($1,">=",$3);$$->add_parse_info(location, linebuf);}
| expr '=' expr {$$ = new Op($1,"=",$3);$$->add_parse_info(location, linebuf);}
| expr '>' expr {$$ = new Op($1,">",$3);$$->add_parse_info(location, linebuf);}
| expr '<' expr {$$ = new Op($1,"<",$3);$$->add_parse_info(location, linebuf);}
| expr "div" expr {$$ = new Op($1,"div",$3);$$->add_parse_info(location, linebuf);}
| expr "mod" expr {$$ = new Op($1,"mod",$3);$$->add_parse_info(location, linebuf);}
| expr "and" expr {$$ = new Op($1,"and",$3);$$->add_parse_info(location, linebuf);}
| expr "or" expr {$$ = new Op($1,"or",$3);$$->add_parse_info(location, linebuf);}
| "not" expr {$$ = new Op("not",$2);$$->add_parse_info(location, linebuf);}
| '+' expr %prec UPLUS {$$ = new Op("+",$2);$$->add_parse_info(location, linebuf);}
| '-' expr %prec UMINUS {$$ = new Op("-",$2);$$->add_parse_info(location, linebuf);}
;

fun_call:
  T_id '('params')' {$$ = new FunctionCall(*$1,$3);$$->add_parse_info(location, linebuf);}
;

proc_call:
  T_id '('params')' {$$ = new ProcCall(*$1,$3);$$->add_parse_info(location, linebuf);}
;

params:
/* nothing */ { $$ = new ExprList();$$->add_parse_info(location, linebuf);}
|mult_exprs {$$ = $1;}
;

mult_exprs:
  expr {$$ = new ExprList($1);$$->add_parse_info(location, linebuf);}
| mult_exprs ',' expr { $1->append($3); $$ = $1;}
;

%%


int main() {
  int result = yyparse();
  return result;
}
